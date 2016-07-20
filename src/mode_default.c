/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016 Delwink, LLC
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

#include <string.h>

#include "colors.h"
#include "font.h"
#include "keys.h"
#include "log.h"
#include "mode_default.h"
#include "tibchar.h"
#include "util.h"

static SDL_Surface *
render_line (const struct tib_expr *line)
{
  int rc;
  char *s = tib_expr_tostr (line);
  if (!s)
    {
      error ("Error converting expression to string");
      return NULL;
    }

  unsigned int len = strlen (s);
  SDL_Surface *parts[len];

  SDL_Rect pos;
  pos.x = 0;
  pos.y = 8;

  for (unsigned int i = 0; i < len; ++i)
    {
      SDL_Surface *part = get_font_char (s[i]);

      pos.x += 6;
      if (pos.x > 96)
        {
          pos.y += 8;
          pos.x = 6;
        }

      parts[i] = part;
    }

  SDL_Surface *final = SDL_CreateRGBSurface (0, 96, pos.y, 32, 0, 0, 0, 0);
  if (!final)
    {
      error ("Failed to initialize expression render surface: %s",
             SDL_GetError ());
      goto end;
    }

  SDL_LockSurface (final);

  rc = SDL_FillRect (final, NULL, SDL_MapRGB (final->format, 255, 255, 255));
  if (rc < 0)
    error ("Failed to set background of expression render surface: %s",
           SDL_GetError ());

  SDL_UnlockSurface (final);

  pos.x = 0;
  pos.y = 0;
  for (unsigned int i = 0; i < len; ++i)
    {
      if (pos.x + 6 > 96)
        {
          pos.x = 0;
          pos.y += 8;
        }

      rc = SDL_BlitSurface (parts[i], NULL, final, &pos);
      if (rc < 0)
        {
          error ("Failed to blit expression portion: %s", SDL_GetError ());
          continue;
        }

      pos.x += 6;
    }

 end:
  free (s);
  return final;
}

static void
draw_line (const struct tib_expr *line, SDL_Surface *final,
           unsigned int *height, bool right_align)
{
  SDL_Surface *line_render = render_line (line);
  if (line_render)
    {
      SDL_Rect pos;
      if (right_align)
        pos.x = 96 - line_render->w;
      else
        pos.x = 0;

      *height += line_render->h;
      pos.y = 64 - *height;

      int rc = SDL_BlitSurface (line_render, NULL, final, &pos);
      if (rc < 0)
        error ("Failed to draw line on screen frame: %s", SDL_GetError ());

      SDL_FreeSurface (line_render);
    }
}

SDL_Surface *
default_draw (const struct screen *screen)
{
  int rc;
  SDL_Surface *final;

  final = SDL_CreateRGBSurface (0, 96, 64, 32, 0, 0, 0, 0);
  if (!final)
    {
      error ("Failed to initialize screen frame: %s", SDL_GetError ());
      return NULL;
    }

  rc = SDL_FillRect (final, NULL, SDL_MapRGB (final->format, 255, 255, 255));
  if (rc < 0)
    error ("Failed to fill screen frame with white background: %s",
           SDL_GetError ());

  struct state *state = screen->state;
  unsigned int height = 0;
  draw_line (&state->entry, final, &height, false);

  for (int i = state->history_len - 1; i >= 0 && height < 64; --i)
    {
      draw_line (&state->answer_strings[i], final, &height, true);

      if (height < 64)
        draw_line (&state->history[i], final, &height, false);
    }

  return final;
}

int
default_input (struct screen *screen, SDL_KeyboardEvent *key)
{
  SDL_Keycode code = key->keysym.sym;
  Uint16 mod = key->keysym.mod;

  if (mod & KMOD_CTRL)
    {
    }
  else if (mod & KMOD_SHIFT)
    {
    }
  else
    {
    }

  int normal = normalize_keycode (code);
  if (normal)
    return entry_write (screen->state, normal);

  return 0;
}
