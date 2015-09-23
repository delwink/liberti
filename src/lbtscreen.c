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

#include "lbtscreen.h"
#include "tiberr.h"

#define foreachscr(R,S) for (S = R; S != NULL; S = S->next)
#define CURSOR self->cursors[self->mode]

struct screen_reg
{
  lbt_Screen *scr;
  struct screen_reg *next;
};

static struct screen_reg *registry = NULL;

lbt_Screen *
lbt_new_Screen (size_t width, size_t height, lbt_State *state)
{
  lbt_Screen *new = malloc (sizeof (lbt_Screen));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->refs = 1;
  new->height = height;
  new->width = width;
  new->mode = LBT_COMMAND_MODE;
  new->lines = NULL;

  new->value = malloc (height * sizeof (bool *));
  if (NULL == new->value)
    {
      free (new);
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  size_t i, j;
  for (i = 0; i < height; ++i)
    {
      new->value[i] = malloc (width * sizeof (bool));
      if (NULL == new->value[i])
	goto fail;

      for (j = 0; j < width; ++j)
	new->value[i][j] = false;
    }

  lbt_State_incref (state);
  new->state = state;

  if (!registry)
    {
      registry = malloc (sizeof (struct screen_reg));
      if (NULL == registry)
	goto fail;

      registry->scr = new;
      registry->next = NULL;
    }
  else
    {
      struct screen_reg *r = registry;
      while (r->next != NULL)
	r = r->next;

      r->next = malloc (sizeof (struct screen_reg));
      r = r->next;
      if (NULL == r)
	goto fail;

      r->scr = new;
      r->next = NULL;
    }

  return new;

 fail:
  for (j = 0; j < i; ++j)
    free (new->value[j]);
  free (new->value);
  free (new);
  tib_errno = TIB_EALLOC;
  return NULL;
}

void
lbt_Screen_incref (lbt_Screen *self)
{
  ++self->refs;
}

void
lbt_Screen_decref (lbt_Screen *self)
{
  if (--self->refs == 0)
    {
      size_t i;
      for (i = 0; i < self->height; ++i)
	free (self->value[i]);
      free (self->value);

      lbt_Screen_clear_lines (self);
      lbt_State_decref (self->state);

      free (self);

      struct screen_reg *r = registry;
      if (r->scr == self)
	{
	  if (r->next)
	    registry = r->next;
	  else
	    registry = NULL;

	  free (r);
	}
      else
	{
	  while (r->next->scr != self)
	    r = r->next;

	  struct screen_reg *temp = r->next->next;
	  free (r->next);
	  r->next = temp;
	}
    }
}

void
lbt_Screen_clear (lbt_Screen *self)
{
  size_t i, j;
  for (i = 0; i < self->height; ++i)
    for (j = 0; j < self->width; ++j)
      self->value[i][j] = false;
}

size_t
lbt_Screen_height (const lbt_Screen *self)
{
  return self->height;
}

size_t
lbt_Screen_width (const lbt_Screen *self)
{
  return self->width;
}

int
lbt_Screen_set (lbt_Screen *self, size_t x, size_t y, bool state)
{
  if (x >= self->width || y >= self->height)
    return TIB_EINDEX;

  self->value[y][x] = state;
  return 0;
}

int
lbt_Screen_toggle (lbt_Screen *self, size_t x, size_t y)
{
  return lbt_Screen_set (self, x, y, !lbt_Screen_get (self, x, y));
}

bool
lbt_Screen_get (const lbt_Screen *self, size_t x, size_t y)
{
  if (x >= self->width || y >= self->height)
    return false;

  return self->value[y][x];
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
lbt_Screen_add_line (lbt_Screen *self, const tib_Expression *text, int64_t x,
		     int64_t y)
{
  if (!self->lines)
    {
      self->lines = new_line (text, x, y);
      if (NULL == self->lines)
	return TIB_EALLOC;

      return 0;
    }

  struct lbt_screen_line *last;
  lbt_foreachline (self, last)
    ; /* set to the last line in the list */

  last->next = new_line (text, x, y);
  if (NULL == last->next)
    return TIB_EALLOC;

  return 0;
}

struct lbt_screen_line *
lbt_Screen_get_line (const lbt_Screen *self, size_t i)
{
  struct lbt_screen_line *line;
  lbt_foreachline (self, line)
    {
      if (0 == i)
	return line;

      --i;
    }

  return NULL;
}

void
lbt_Screen_del_line (lbt_Screen *self, size_t i)
{
  struct lbt_screen_line *temp, *before = lbt_Screen_get_line (self, i-1);

  if (NULL == before)
    {
      if (0 == i && self->lines)
	{
	  temp = self->lines;
	  self->lines = temp->next;
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
lbt_Screen_clear_lines (lbt_Screen *self)
{
  struct lbt_screen_line *line;
  while ((line = self->lines))
    lbt_Screen_del_line (self, 0);
}

size_t
lbt_Screen_num_lines (const lbt_Screen *self)
{
  struct lbt_screen_line *line;
  size_t i = 0;
  lbt_foreachline (self, line)
    ++i;

  return i;
}

void
lbt_Screen_set_state (lbt_Screen *self, lbt_State *state)
{
  /* increase ref count of this one first to prevent freeing if it's the
     same state */
  lbt_State_incref (state);

  lbt_State_decref (self->state);
  self->state = state;
}

int
lbt_Screen_refresh (lbt_Screen *self)
{
  return lbt_Screen_set_mode (self, self->mode);
}

int64_t
to_bounds (int64_t x, int64_t min, int64_t max)
{
  if (x < min)
    x = min;
  else if (x > max)
    x = max;

  return x;
}

static struct lbt_screen_line *
current_line (lbt_Screen *self)
{
  switch (self->mode)
    {
    case LBT_COMMAND_MODE:
      return lbt_Screen_get_line (self, CURSOR.y);

    default:
      return NULL;
    }
}

void
lbt_Screen_move_cursor (lbt_Screen *self, int64_t x, int64_t y)
{
  CURSOR.x += x;
  CURSOR.y += y;

  switch (self->mode)
    {
    case LBT_COMMAND_MODE:
      CURSOR.y = to_bounds (CURSOR.y, 0, (int64_t) lbt_Screen_num_lines (self));

      struct lbt_screen_line *line = current_line (self);
      if (line)
	CURSOR.x = to_bounds (CURSOR.x, 0, tib_Expression_len (line->value));
      break;

    default:
      break;
    }
}

int
lbt_Screen_write_char (lbt_Screen *self, int c)
{
  struct lbt_screen_line *line = current_line (self);

  int64_t x = CURSOR.x;
  if (NULL == line || x < 0 || (size_t) x >= line->value->len)
    return TIB_EINDEX;

  line->value->value[x] = c;
  return 0;
}

int
lbt_Screen_insert_char (lbt_Screen *self, int c)
{
  struct lbt_screen_line *line = current_line (self);
  if (NULL == line)
    return TIB_EINDEX;

  return tib_Expression_insert (line->value, (size_t) CURSOR.x, c);
}

int
lbt_Screen_set_mode (lbt_Screen *self, enum lbt_screen_mode mode)
{
  lbt_Screen_clear (self);
  lbt_Screen_clear_lines (self);

  self->mode = mode;

  char cfgpath[64];
  switch (mode)
    {
    case LBT_COMMAND_MODE:
      sprintf (cfgpath, "mode.[%d].lines", mode);
      config_setting_t *lines = config_lookup (&self->state->conf, cfgpath);

      size_t i = 0;
      config_setting_t *line;
      while ((line = config_setting_get_elem (lines, i)))
	{
	  config_setting_t *setting = config_setting_get_member (line, "x");
	  int64_t x = setting ? config_setting_get_int64 (setting) : 0;

	  setting = config_setting_get_member (line, "y");
	  int64_t y = setting ? config_setting_get_int64 (setting) : 0;

	  setting = config_setting_get_member (line, "text");
	  const char *s = setting ? config_setting_get_string (setting) : "";

	  tib_Expression *text = tib_new_Expression ();
	  if (NULL == text)
	    return TIB_EALLOC;

	  tib_errno = tib_Expression_set (text, s);
	  if (tib_errno)
	    {
	      tib_Expression_decref (text);
	      return tib_errno;
	    }

	  tib_errno = lbt_Screen_add_line (self, text, x, y);
	  tib_Expression_decref (text);
	  if (tib_errno)
	    return tib_errno;

	  self->cursors[LBT_COMMAND_MODE].x = x;
	  self->cursors[LBT_COMMAND_MODE].y = 0;
	}
      return 0;

    default:
      return 0;
    }
}

enum lbt_screen_mode
lbt_Screen_get_mode (const lbt_Screen *self)
{
  return self->mode;
}
