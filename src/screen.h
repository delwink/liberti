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

#ifndef DELWINK_LIBERTI_SCREEN_H
#define DELWINK_LIBERTI_SCREEN_H

#include <SDL.h>

#include "point.h"
#include "state.h"

enum screen_mode
  {
    DEFAULT_SCREEN_MODE,
    NUM_SCREEN_MODES
  };

struct screen
{
  struct state *state;

  struct point2d pos;
  struct point2d size;

  enum screen_mode mode;
};

SDL_Surface *
screen_draw (const struct screen *screen);

#endif
