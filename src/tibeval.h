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

#ifndef DELWINK_TIB_EVAL_H
#define DELWINK_TIB_EVAL_H

#include <stdbool.h>

#include "tibexpr.h"
#include "tibtype.h"

bool
is_sign_operator (int c);

bool
is_math_operator (int c);

unsigned int
sign_count (const struct tib_expr *expr);

bool
contains_i (const struct tib_expr *expr);

TIB *
tib_eval (const struct tib_expr *expr);

int
tib_eval_surrounded (const struct tib_expr *expr);

bool
tib_eval_isnum (const struct tib_expr *expr);

bool
tib_eval_isstr (const struct tib_expr *expr);

bool
tib_eval_islist (const struct tib_expr *expr);

bool
tib_eval_ismatrix (const struct tib_expr *expr);

int
tib_eval_close_parens (struct tib_expr *expr);

#endif
