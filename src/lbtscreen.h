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

#ifndef DELWINK_LIBLIBERTI_SCREEN_H
#define DELWINK_LIBLIBERTI_SCREEN_H

#include <stdint.h>
#include <stdlib.h>

#include "lbtstate.h"
#include "tibexpr.h"

#define lbt_foreachline(S,L) for (L = S->state->lines[S->mode]; L != NULL; \
				  L = L->next)

#define lbt_foreachline_rev(S,L) for (L = S->state->last_lines[S->mode]; \
				      L != NULL; L = L->prev)

typedef int (*lbt_ScreenTrigger) (void *data);

struct lbt_cursor_pos
{
  int64_t x;
  int64_t y;
};

typedef struct lbt_screen_trigger_list
{
  lbt_ScreenTrigger trigger;
  struct lbt_screen_trigger_list *next;
} lbt_ScreenTriggerList;

typedef struct
{
  size_t refs;

  enum lbt_screen_mode mode;
  struct lbt_cursor_pos cursor;
  lbt_ScreenTriggerList *triggers[LBT_NUM_MODES];
  lbt_State *state;
} lbt_Screen;

lbt_Screen *
lbt_new_Screen (lbt_State *state);

void
lbt_Screen_incref (lbt_Screen *self);

void
lbt_Screen_decref (lbt_Screen *self);

int
lbt_Screen_add_line (lbt_Screen *self, const tib_Expression *text, int64_t x,
		     int64_t y);

struct lbt_screen_line *
lbt_Screen_get_line (const lbt_Screen *self, size_t i);

void
lbt_Screen_del_line (lbt_Screen *self, size_t i);

void
lbt_Screen_clear_lines (lbt_Screen *self);

size_t
lbt_Screen_num_lines (const lbt_Screen *self);

void
lbt_Screen_set_state (lbt_Screen *self, lbt_State *state);

void
lbt_Screen_refresh (lbt_Screen *self);

struct lbt_screen_line *
lbt_Screen_current_line (const lbt_Screen *self);

void
lbt_Screen_move_cursor (lbt_Screen *self, int64_t x, int64_t y);

int
lbt_Screen_write_char (lbt_Screen *self, int c);

int
lbt_Screen_insert_char (lbt_Screen *self, int c);

/* modes */

void
lbt_Screen_set_mode (lbt_Screen *self, enum lbt_screen_mode mode);

enum lbt_screen_mode
lbt_Screen_get_mode (const lbt_Screen *self);

#endif
