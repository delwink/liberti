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

#include "button.h"
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
  SDL_Surface *parts[len + 1];
  parts[len] = get_font_char (' ');

  SDL_Rect pos;
  pos.x = 0;
  pos.y = 8;

  for (unsigned int i = 0; i < len; ++i)
    {
      SDL_Surface *part = get_font_char (s[i]);

      pos.x += 6;
      if (pos.x >= 96)
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
  for (unsigned int i = 0; i <= len; ++i)
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
        pos.x = 96 - (line->len * 6);
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

static void
draw_cursor (const struct state *state, SDL_Surface *frame)
{
  if (!state->blink_state)
    return;

  SDL_Keymod mod = SDL_GetModState ();
  int c = 1;

  if (state->insert_mode)
    c += 4;

  if (STATE_2ND == state->action_state || mod & KMOD_CTRL)
    ++c;
  else if (STATE_ALPHA == state->action_state || mod & KMOD_SHIFT)
    c += 2;

  SDL_Rect pos = { .x = (6 * (state->entry_cursor % 16)), .y = (64 - 8) };
  SDL_Surface *tile = get_font_char (c);

  int rc = SDL_BlitSurface (tile, NULL, frame, &pos);
  if (rc < 0)
    error ("Failed to draw cursor on screen frame: %s", SDL_GetError ());
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
  draw_cursor (state, final);

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
  struct state *state = screen->state;
  SDL_Keycode code = key->keysym.sym;
  SDL_Keymod mod = key->keysym.mod;

  switch (state->action_state)
    {
    case STATE_2ND:
      mod |= KMOD_LCTRL;
      break;

    case STATE_ALPHA:
      mod |= KMOD_LSHIFT;
      break;

    default:
      break;
    }

  switch (code)
    {
    case SDLK_BACKSPACE:
      if (!state->entry_cursor)
        return 0;

      --state->entry_cursor;
      // SPILLS OVER!
    case SDLK_DELETE:
      if (state->entry_cursor < state->entry.len)
        tib_expr_delete (&state->entry, state->entry_cursor);
      return 0;

    case SDLK_INSERT:
      state->insert_mode = !state->insert_mode;
      return 0;

    case SDLK_LEFT:
      entry_move_cursor (state, LEFT);
      return 0;

    case SDLK_RETURN:
      if (mod & KMOD_SHIFT)
        return entry_recall (state);
      else
        return state_calc_entry (state);

    case SDLK_RIGHT:
      entry_move_cursor (state, RIGHT);
      return 0;
    }

  int normal = normalize_keycode (code, mod);
  if (normal)
    return entry_write (state, normal);

  return 0;
}
