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
#include "ttf.h"

enum button_action_type
  {
    CHANGE_MODES,
    CHAR_INSERT,
    CURSOR_MOVE,
    TOGGLE_2ND,
    TOGGLE_ALPHA,
    TOGGLE_INSERT
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
    UP    = -16,
    DOWN  =  16,
    LEFT  =  -1,
    RIGHT =   1
  };

union button_action
{
  int char_insert;
  enum cursor_direction cursor_move;
  enum screen_mode mode_open;
};

struct button_action_set
{
  enum button_action_type type;
  union button_action which;
};

struct skin_button
{
  struct button_action_set actions[NUM_SCREEN_MODES][NUM_ACTION_STATES];
  struct point2d pos;
  struct point2d size;
};

struct skin_button_list
{
  struct skin_button *button;
  struct skin_button_list *next;
};

struct skin_render_cache
{
  SDL_Surface *surface;
  struct skin_render_cache *next;
};

struct skin_screen_list
{
  struct screen screen;
  struct skin_screen_list *next;
};

typedef struct
{
  SDL_Surface *background;
  struct skin_render_cache *renders;

  struct screen *active_screen;
  struct skin_screen_list *screens;
  struct skin_button_list *buttons;

  struct point2d size;
  enum button_action_state action_state;
  bool insert_mode;
} Skin;

Skin *
open_skin (const char *path, struct state *state, struct point2d size);

void
free_skin (Skin *self);

int
Skin_click (Skin *self, struct point2d pos);

SDL_Surface *
Skin_get_frame (Skin *self, const struct fontset *fonts);

#endif
