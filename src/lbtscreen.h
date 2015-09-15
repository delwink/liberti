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

typedef struct
{
  size_t refs;
  size_t height;
  size_t width;
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

bool
lbt_Screen_get (const lbt_Screen *self, size_t x, size_t y);

#endif
