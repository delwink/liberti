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

#ifndef DELWINK_LIBERTI_SKIN_H
#define DELWINK_LIBERTI_SKIN_H

#include <SDL.h>
#include <stdlib.h>

#include "lbtscreen.h"
#include "menu.h"
#include "point.h"

#define DEFAULT_MODE LBT_NUM_MODES

enum button_action_type
  {
    CHAR_INSERT,
    CURSOR_MOVE,
    MENU_OPEN,
    TOGGLE_2ND,
    TOGGLE_ALPHA
  };

enum button_action_state
  {
    STATE_NORMAL,
    STATE_2ND,
    STATE_ALPHA,
    NUM_ACTION_STATES
  };

enum cursor_direction
  {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };

union button_action
{
  int char_insert;
  enum cursor_direction cursor_move;
  Menu menu_open;
};

struct skin_button
{
  union button_action actions[LBT_NUM_MODES+1][NUM_ACTION_STATES];
  struct point2d size;
};

struct skin_button_list
{
  struct point2d pos;
  struct skin_button *button;
  struct skin_button_list *next;
};

struct skin_render_cache
{
  SDL_Surface *surface;
  struct skin_render_cache *next;
};

struct skin_screen_render_cache
{
  struct skin_render_cache *renders;
  struct skin_screen_render_cache *next;
};

struct skin_screen_list
{
  lbt_Screen *screen;
  struct point2d pos;
  struct skin_screen_list *next;
};

typedef struct
{
  SDL_Surface *background;
  size_t active_screen;
  struct skin_button_list *buttons;
  struct skin_render_cache *full_renders;
  struct skin_screen_render_cache *partial_renders;
  struct skin_screen_list *screens;
} Skin;

Skin *
open_skin (const char *path, lbt_State *state);

void
free_skin (Skin *self);

int
Skin_click (Skin *self, struct point2d pos);

SDL_Surface *
Skin_get_frame (const Skin *self);

#endif
