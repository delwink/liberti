/*
 *  LiberTI - Libre TI calculator emulator designed for LibreCalc
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

#include "skin.h"
#include "tiberr.h"

#define DEFAULT_SKIN				\
  "screens=[{"					\
  "mode=\"command\";"				\
  "x=0L;"					\
  "y=0L;"					\
  "}];"
  

static int
mode_from_string (const char *s)
{
  if (0 == strcmp (s, "command"))
    return LBT_COMMAND_MODE;

  return -1;
}

static int
add_screen (Skin *self, lbt_State *state, struct point2d pos,
	    enum lbt_screen_mode mode)
{
  struct skin_screen_list *node = self->screens, *prev = NULL;

  if (NULL == node)
    {
      self->screens = malloc (sizeof (struct skin_screen_list));
      if (NULL == self->screens)
	return TIB_EALLOC;

      node = self->screens;
    }
  else
    {
      while (node->next)
	node = node->next;

      node->next = malloc (sizeof (struct skin_screen_list));
      if (NULL == node->next)
	return TIB_EALLOC;

      prev = node;
      node = node->next;
    }

  node->screen = lbt_new_Screen (state);
  if (NULL == node->screen)
    goto fail;

  node->pos = pos;
  lbt_Screen_set_mode (node->screen, mode);

  return 0;

 fail:
  if (prev)
    prev->next = NULL;
  lbt_Screen_decref (node->screen);
  return tib_errno;
}

Skin *
open_skin (const char *data, lbt_State *state)
{
  if (NULL == state)
    {
      tib_errno = TIB_ENULLPTR;
      return NULL;
    }

  Skin *new = malloc (sizeof (Skin));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->active_screen = 0;
  new->buttons = NULL;
  new->full_renders = NULL;
  new->partial_renders = NULL;
  new->screens = NULL;

  if (!data)
    data = DEFAULT_SKIN;

  config_t conf;
  config_init (&conf);

  tib_errno = config_read_string (&conf, data);
  if (CONFIG_FALSE == tib_errno)
    {
      tib_errno = TIB_EBADFILE;
      goto fail;
    }

  tib_errno = 0;

  config_setting_t *setting = config_lookup (&conf, "screens");
  if (setting)
    {
      if (!config_setting_is_array (setting))
	{
	  tib_errno = TIB_EBADFILE;
	  goto fail;
	}

      const config_setting_t *screens = setting;
      unsigned int i = 0;
      while ((setting = config_setting_get_elem (screens, i++)))
	{
	  if (!config_setting_is_group (setting))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  const config_setting_t *screen = setting;

	  setting = config_setting_get_member (screen, "mode");
	  if (!setting || config_setting_type (setting) != CONFIG_TYPE_STRING)
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  int mode = mode_from_string (config_setting_get_string (setting));
	  if (-1 == mode)
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  setting = config_setting_get_member (screen, "x");
	  if (!setting
	      || (config_setting_type (setting) != CONFIG_TYPE_INT64
		  && config_setting_type (setting) != CONFIG_TYPE_INT))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  struct point2d pos;
	  pos.x = config_setting_get_int64 (setting);

	  setting = config_setting_get_member (screen, "y");
	  if (!setting
	      || (config_setting_type (setting) != CONFIG_TYPE_INT64
		  && config_setting_type (setting) != CONFIG_TYPE_INT))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  pos.y = config_setting_get_int64 (setting);

	  tib_errno = add_screen (new, state, pos, mode);
	  if (tib_errno)
	    goto fail;
	}
    }

  return new;

 fail:
  config_destroy (&conf);

  if (new->screens)
    {
      struct skin_screen_list *s = new->screens;
      while (s->next)
	{
	  lbt_Screen_decref (s->screen);
	  s = s->next;
	  free (new->screens);
	  new->screens = s;
	}
    }

  free (new);
  return NULL;
}

static void
free_render_cache (struct skin_render_cache *c)
{
  while (c)
    {
      struct skin_render_cache *temp = c;
      SDL_FreeSurface (temp->surface);

      c = c->next;
      free (temp);
    }
}

void
free_skin (Skin *self)
{
  struct skin_screen_list *s = self->screens;
  while (s)
    {
      struct skin_screen_list *temp = s;
      lbt_Screen_decref (s->screen);

      s = s->next;
      free (temp);
    }

  self->active_screen = 0;

  if (self->buttons)
    {
      free (self->buttons);
      self->buttons = NULL;
    }

  free_render_cache (self->full_renders);
  self->full_renders = NULL;

  if (self->partial_renders)
    {
      struct skin_screen_render_cache *c = self->partial_renders;
      while (c)
	{
	  struct skin_screen_render_cache *temp = c;
	  free_render_cache (c->renders);

	  c = c->next;
	  free (temp);
	}
    }
}
