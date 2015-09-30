/*
 *  libliberti - Backend functionality for LiberTI
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

#ifndef DELWINK_LIBLIBERTI_STATE_H
#define DELWINK_LIBLIBERTI_STATE_H

#include <stdint.h>
#include <stdlib.h>

#include "tibexpr.h"

enum lbt_screen_mode
  {
    LBT_COMMAND_MODE,
    LBT_NUM_MODES /* this will contain the number of mode IDs defined */
  };

struct lbt_screen_line
{
  tib_Expression *value;
  int64_t x;
  int64_t y;

  struct lbt_screen_line *next;
};

typedef struct
{
  size_t refs;

  char *save_path;
  struct lbt_screen_line *lines[LBT_NUM_MODES];
} lbt_State;

lbt_State *
lbt_new_State (const char *save_path);

void
lbt_State_incref (lbt_State *self);

void
lbt_State_decref (lbt_State *self);

int
lbt_State_add_line (lbt_State *self, const tib_Expression *text, int64_t x,
		    int64_t y, enum lbt_screen_mode mode);

struct lbt_screen_line *
lbt_State_get_line (const lbt_State *self, size_t i,
		    enum lbt_screen_mode mode);

void
lbt_State_del_line (lbt_State *self, size_t i, enum lbt_screen_mode mode);

void
lbt_State_clear_lines (lbt_State *self, enum lbt_screen_mode mode);

void
lbt_State_clear_all_lines (lbt_State *self);

size_t
lbt_State_num_lines (const lbt_State *self, enum lbt_screen_mode mode);

#endif
