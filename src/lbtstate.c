/*
 *  libliberti - Backend functionality for LiberTI
 *  Copyright (C) 2015 Delwink, LLC
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, version 3 only.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libconfig.h>
#include <stdio.h>
#include <string.h>

#include "lbtstate.h"
#include "tibchar.h"
#include "tiberr.h"

lbt_State *
lbt_new_State (const char *save_path)
{
  lbt_State *new = malloc (sizeof (lbt_State));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->save_path = malloc ((strlen (save_path) + 1) * sizeof (char));
  if (NULL == new->save_path)
    {
      free (new);
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->refs = 1;
  strcpy (new->save_path, save_path);

  config_t conf;
  config_init (&conf);

  FILE *save = fopen (save_path, "r");
  if (save)
    {
      int rc = config_read (&conf, save);
      fclose (save);
      if (!rc)
	{
	  config_destroy (&conf);
	  free (new->save_path);
	  free (new);
	  tib_errno = TIB_EBADFILE;
	  return NULL;
	}
    }

  config_setting_t *setting = config_lookup (&conf, "mode.command.lines");
  if (setting)
    {
      size_t i = 0;
      config_setting_t *line;
      while ((line = config_setting_get_elem (setting, i++)))
	{
	  config_setting_t *e = config_setting_get_member (line, "x");
	  if (NULL == e)
	    continue;
	  int64_t x = config_setting_get_int64 (e);

	  e = config_setting_get_member (line, "y");
	  if (NULL == e)
	    continue;
	  int64_t y = config_setting_get_int64 (e);

	  e = config_setting_get_member (line, "text");
	  if (NULL == e)
	    continue;
	  const char *s = config_setting_get_string (line);

	  tib_Expression *text = tib_encode_str (s);
	  if (NULL == text)
	    goto fail;

	  tib_errno = lbt_State_add_line (new, text, x, y, LBT_COMMAND_MODE);
	  tib_Expression_decref (text);
	  if (tib_errno)
	    goto fail;
	}
    }

  config_destroy (&conf);

  return new;

 fail:
  lbt_State_clear_all_lines (new);
  config_destroy (&conf);
  free (new->save_path);
  free (new);
  return NULL;
}

void
lbt_State_incref (lbt_State *self)
{
  ++self->refs;
}

void
lbt_State_decref (lbt_State *self)
{
  if (--self->refs == 0)
    {
      config_t conf;
      config_init (&conf);

      config_setting_t *setting = config_root_setting (&conf);

#define ADD(S,N,T) S = config_setting_add (S, N, T); if (NULL == S) goto clean;

      ADD (setting, "mode", CONFIG_TYPE_GROUP);
      ADD (setting, "command", CONFIG_TYPE_GROUP);
      ADD (setting, "lines", CONFIG_TYPE_ARRAY);

      config_setting_t *info;
      struct lbt_screen_line *line;
      while ((line = self->lines[LBT_COMMAND_MODE]))
	{
	  info = config_setting_add (setting, NULL, CONFIG_TYPE_GROUP);
	  if (NULL == info)
	    goto clean;

	  ADD (info, "x", CONFIG_TYPE_INT64);
	  config_setting_set_int64 (info, line->x);

	  info = config_setting_parent (info);
	  ADD (info, "y", CONFIG_TYPE_INT64);
	  config_setting_set_int64 (info, line->y);

	  info = config_setting_parent (info);
	  ADD (info, "text", CONFIG_TYPE_STRING);
	  char *text = tib_Expression_as_str (line->value);
	  if (NULL == text)
	    goto clean;

	  tib_errno = config_setting_set_string (info, text);
	  free (text);
	  if (CONFIG_FALSE == tib_errno)
	    goto clean;
	  tib_errno = 0;

	  lbt_State_del_line (self, 0, LBT_COMMAND_MODE);
	}

#undef ADD

      config_write_file (&conf, self->save_path);

    clean:
      config_destroy (&conf);
      lbt_State_clear_all_lines (self);
      free (self->save_path);
      free (self);
    }
}

static struct lbt_screen_line *
new_line (const tib_Expression *text, size_t x, size_t y)
{
  struct lbt_screen_line *new = malloc (sizeof (struct lbt_screen_line));
  if (NULL == new)
    return NULL;

  new->value = tib_copy_Expression (text);
  if (NULL == new->value)
    {
      free (new);
      return NULL;
    }

  new->x = x;
  new->y = y;
  new->next = NULL;

  return new;
}

int
lbt_State_add_line (lbt_State *self, const tib_Expression *text, int64_t x,
		    int64_t y, enum lbt_screen_mode mode)
{
  if (!self->lines[mode])
    {
      self->lines[mode] = new_line (text, x, y);
      if (NULL == self->lines[mode])
	return TIB_EALLOC;

      return 0;
    }

  struct lbt_screen_line *last;
  for (last = self->lines[mode]; last->next != NULL; last = last->next)
    ; /* set to the last line in the list */

  last->next = new_line (text, x, y);
  if (NULL == last->next)
    return TIB_EALLOC;

  return 0;
}

struct lbt_screen_line *
lbt_State_get_line (const lbt_State *self, size_t i, enum lbt_screen_mode mode)
{
  struct lbt_screen_line *line;
  for (line = self->lines[mode]; line != NULL; line = line->next)
    {
      if (0 == i)
	return line;

      --i;
    }

  return NULL;
}

void
lbt_State_del_line (lbt_State *self, size_t i, enum lbt_screen_mode mode)
{
  struct lbt_screen_line *temp, *before = lbt_State_get_line (self, i-1, mode);

  if (NULL == before)
    {
      if (0 == i && self->lines[mode])
	{
	  temp = self->lines[mode];
	  self->lines[mode] = temp->next;
	}
      else
	{
	  return;
	}
    }
  else if (NULL == before->next)
    {
      return;
    }
  else
    {
      temp = before->next;
      before->next = temp->next;
    }

  tib_Expression_decref (temp->value);
  free (temp);
}

void
lbt_State_clear_lines (lbt_State *self, enum lbt_screen_mode mode)
{
  struct lbt_screen_line *line;
  while ((line = self->lines[mode]))
    lbt_State_del_line (self, 0, mode);
}

void
lbt_State_clear_all_lines (lbt_State *self)
{
  enum lbt_screen_mode mode;
  for (mode = LBT_COMMAND_MODE; mode < LBT_NUM_MODES; ++mode)
    lbt_State_clear_lines (self, mode);
}

size_t
lbt_Screen_num_lines (const lbt_State *self, enum lbt_screen_mode mode)
{
  struct lbt_screen_line *line;
  size_t i = 0;
  for (line = self->lines[mode]; line != NULL; line = line->next)
    ++i;

  return i;
}
