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

struct lbt_screen_dim
{
  size_t height;
  size_t width;
};

int
lbt_screen_init (struct lbt_screen_dim dims);

void
lbt_screen_free (void);

void
lbt_clear_screen (void);

int
lbt_set_pixel (size_t x, size_t y, bool state);

bool
lbt_get_pixel (size_t x, size_t y);

#endif
