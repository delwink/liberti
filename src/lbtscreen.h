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

#include <stdbool.h>
#include <stdlib.h>

#include "tibexpr.h"

#define lbt_foreachline(S,L) for (L = S->lines; L != NULL; L = L->next)

enum lbt_screen_mode
  {
    LBT_COMMAND_MODE
  };

struct lbt_screen_line
{
  tib_Expression *value;
  size_t x;
  size_t y;

  struct lbt_screen_line *next;
};

typedef struct
{
  size_t refs;
  size_t height;
  size_t width;

  enum lbt_screen_mode mode;
  struct lbt_screen_line *lines;
  bool **value;
} lbt_Screen;

lbt_Screen *
lbt_new_Screen (size_t width, size_t height);

void
lbt_Screen_incref (lbt_Screen *self);

void
lbt_Screen_decref (lbt_Screen *self);

void
lbt_Screen_clear (lbt_Screen *self);

size_t
lbt_Screen_height (const lbt_Screen *self);

size_t
lbt_Screen_width (const lbt_Screen *self);

int
lbt_Screen_set (lbt_Screen *self, size_t x, size_t y, bool state);

int
lbt_Screen_toggle (lbt_Screen *self, size_t x, size_t y);

bool
lbt_Screen_get (const lbt_Screen *self, size_t x, size_t y);

int
lbt_Screen_add_line (lbt_Screen *self, const tib_Expression *text, size_t x,
		     size_t y);

struct lbt_screen_line *
lbt_Screen_get_line (const lbt_Screen *self, size_t i);

void
lbt_Screen_del_line (lbt_Screen *self, size_t i);

void
lbt_Screen_clear_lines (lbt_Screen *self);

/* modes */

int
lbt_Screen_set_mode (lbt_Screen *self, enum lbt_screen_mode mode);

enum lbt_screen_mode
lbt_Screen_get_mode (const lbt_Screen *self);

#endif
