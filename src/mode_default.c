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

#include "colors.h"
#include "log.h"
#include "mode_default.h"
#include "tibchar.h"

static SDL_Surface *
render_line (const struct tib_expr *line, TTF_Font *font)
{
  int rc, w, h, font_height;
  SDL_Surface **parts = malloc (line->len * sizeof (SDL_Surface *));
  if (!parts)
    {
      error ("Failed to initialize expression portions array");
      return NULL;
    }

  font_height = TTF_FontHeight (font);
  w = 0;
  h = font_height;

  unsigned int i;
  tib_expr_foreach (line, i)
    {
      int c = line->data[i];
      const char *s = tib_special_char_text (c);
      SDL_Surface *part;

      if (s)
        {
          part = TTF_RenderUTF8_Solid (font, s, BLACK);
        }
      else
        {
          char single[2];
          single[0] = c;
          single[1] = '\0';

          part = TTF_RenderUTF8_Solid (font, single, BLACK);
        }

      if (!part)
        {
          error ("Failed to render expression portion: %s", TTF_GetError ());
          parts[i] = NULL;
          continue;
        }

      w += part->w;
      if (w > 96)
        {
          h += font_height;
          w -= part->w;
        }

      parts[i] = part;
    }

  SDL_Surface *final = SDL_CreateRGBSurface (0, w, h, 32, 0, 0, 0, 0);
  if (!final)
    {
      error ("Failed to initialize expression render surface: %s",
             SDL_GetError ());
      goto fail;
    }

  SDL_LockSurface (final);

  rc = SDL_FillRect (final, NULL, SDL_MapRGBA (final->format, 0, 0, 0, 0));
  if (rc < 0)
    error ("Failed to set background of expression render surface: %s",
           SDL_GetError ());

  w = 0;
  h = 0;
  for (i = 0; i < line->len; ++i)
    {
      if (w + parts[i]->w > 96)
        {
          w = 0;
          h += font_height;
        }

      SDL_Rect pos;
      pos.x = w;
      pos.y = h;

      rc = SDL_BlitSurface (parts[i], NULL, final, &pos);
      if (rc < 0)
        {
          error ("Failed to blit expression portion: %s", SDL_GetError ());
          continue;
        }

      w += parts[i]->w;
    }

  SDL_UnlockSurface (final);

 fail:
  for (i = 0; i < line->len; ++i)
    if (parts[i])
      SDL_FreeSurface (parts[i]);

  free (parts);

  return final;
}

static void
draw_line (const struct tib_expr *line, SDL_Surface *final,
           const struct fontset *fonts, unsigned int *height, bool right_align)
{
  SDL_Surface *line_render = render_line (line, fonts->reg);
  if (line_render)
    {
      SDL_Rect pos;
      if (right_align)
        pos.x = 96 - line_render->w;
      else
        pos.x = 0;

      pos.y = 64 - *height;

      int rc = SDL_BlitSurface (line_render, NULL, final, &pos);
      if (rc < 0)
        error ("Failed to draw line on screen frame: %s", SDL_GetError ());

      *height += line_render->h;
      SDL_FreeSurface (line_render);
    }
}

SDL_Surface *
default_draw (const struct screen *screen, const struct fontset *fonts)
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
  draw_line (&state->entry, final, fonts, &height, false);

  for (int i = state->history_len - 1; i >= 0 && height < 64; --i)
    {
      draw_line (&state->answer_strings[i], final, fonts, &height, true);

      if (height < 64)
        draw_line (&state->history[i], final, fonts, &height, false);
    }

  return final;
}

int
default_input (struct screen *screen, SDL_KeyboardEvent *key)
{
  return 0;
}
