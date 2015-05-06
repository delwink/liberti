/*
 *  libtibasic - Read, write, and evaluate TI BASIC programs
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

#ifndef DELWINK_TIB_EVAL_H
#define DELWINK_TIB_EVAL_H

#include <stdbool.h>

#include "tibexpr.h"
#include "tibtype.h"

TIB *
eval (tib_Expression expr);

bool
tib_eval_surrounded (tib_Expression *expr);

bool
tib_eval_surrounded_function (tib_Expression *expr, int function);

bool
tib_eval_isnum (tib_Expression *expr);

bool
tib_eval_isstr (tib_Expression *expr);

bool
tib_eval_islist (tib_Expression *expr);

bool
tib_eval_ismatrix (tib_Expression *expr);

int
tib_eval_find_outside_parens (tib_Expression *expr, int c);

void
tib_eval_fix_parens (tib_Expression **out);

tib_Expression *
tib_eval_parse_commas (tib_Expression *expr);

#endif
