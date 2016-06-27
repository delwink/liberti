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

#include <stdlib.h>

#include "mode_default.h"
#include "screen.h"

struct _screen_mode
{
  int (*draw) (struct screen *);
  int (*input) (struct screen *, int);
};

const struct _screen_mode SCREEN_MODES[NUM_SCREEN_MODES] =
  {
    {
      .draw = default_draw,
      .input = default_input
    }
  };

void
screen_init (struct screen *screen, struct state *state)
{
  static const struct point2d ZERO = { .x = 0, .y = 0 };

  screen->state = state;
  screen->surface = NULL;
  screen->pos = ZERO;
  screen->size = ZERO;
  screen->mode = DEFAULT_SCREEN_MODE;
}

void
screen_destroy (struct screen *screen)
{
  if (screen->surface)
    SDL_FreeSurface (screen->surface);
}
