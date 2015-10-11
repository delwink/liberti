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

#define DEFAULT_SKIN ""

Skin *
open_skin (const char *data)
{
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

  return new;

 fail:
  config_destroy (&conf);
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
