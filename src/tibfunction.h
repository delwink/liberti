/*
 *  libtib - Read, write, and evaluate TI BASIC programs
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

#ifndef DELWINK_TIB_FUNCTION_H
#define DELWINK_TIB_FUNCTION_H

#include <stdbool.h>

#include "tibexpr.h"
#include "tibtype.h"

typedef TIB *(*tib_Function) (const struct tib_expr *);

int
tib_registry_init (void);

void
tib_registry_free (void);

int
tib_registry_add (int key, tib_Function f);

bool
tib_is_func (int key);

TIB *
tib_call (int key, const struct tib_expr *expr);

#endif
