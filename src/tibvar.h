/*
 *  libtib - Read, write, and evaluate TI BASIC programs
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

#ifndef DELWINK_TIB_VAR_H
#define DELWINK_TIB_VAR_H

#include <stdbool.h>

#include "tibtype.h"

typedef struct
{
  int key;
  TIB *value;
} tib_Variable;

void
tib_var_free (void);

int
tib_var_set (int key, const TIB *value);

TIB *
tib_var_get (int key);

bool
tib_is_var (int key);

#endif
