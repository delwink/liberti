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

#include <stdlib.h>

#include "menu.h"
#include "point.h"

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

typedef struct
{
  union button_action actions[NUM_ACTION_STATES];
  struct point2d size;
} SkinButton;

struct skin_button_list
{
  SkinButton *button;
  struct point2d pos;
  struct skin_button_list *next;
};

typedef struct
{
  lbt_Screen *screens;
  size_t num_screens;
  struct point2d *screen_pos;
  struct skin_button_list *buttons;
} Skin;

#endif
