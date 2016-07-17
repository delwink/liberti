/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2015-2016 Delwink, LLC
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

#ifndef DELWINK_LIBERTI_SKIN_H
#define DELWINK_LIBERTI_SKIN_H

#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>

#include "point.h"
#include "screen.h"

struct skin_button_list
{
  struct button *button;
  struct skin_button_list *next;
};

struct skin_screen_list
{
  struct screen screen;
  struct skin_screen_list *next;
};

typedef struct
{
  SDL_Surface *background;
  struct screen *active_screen;
  struct skin_screen_list *screens;
  struct skin_button_list *buttons;
  struct state *state;

  struct point2d size;
} Skin;

Skin *
open_skin (const char *path, struct state *state, struct point2d size);

void
free_skin (Skin *self);

int
Skin_click (Skin *self, struct point2d pos);

int
Skin_input (Skin *self, SDL_KeyboardEvent *event);

SDL_Surface *
Skin_get_frame (Skin *self);

#endif
