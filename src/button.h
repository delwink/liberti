/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016-2017 Delwink, LLC
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

#ifndef DELWINK_LIBERTI_BUTTON_H
#define DELWINK_LIBERTI_BUTTON_H

#include "screen.h"

enum cursor_direction
{
	UP    = -16,
	DOWN  = 16,
	LEFT  = -1,
	RIGHT = 1
};

enum button_action_type
{
	CHANGE_MODES,
	CHAR_INSERT,
	CURSOR_MOVE,
	TOGGLE_2ND,
	TOGGLE_ALPHA,
	TOGGLE_INSERT
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

struct button
{
	struct button_action_set actions[NUM_SCREEN_MODES][NUM_ACTION_STATES];
	struct point2d pos;
	struct point2d size;
};

#endif
