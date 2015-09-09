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

#ifndef DELWINK_TIB_EVAL_H
#define DELWINK_TIB_EVAL_H

#include <stdbool.h>

#include "tibexpr.h"
#include "tibtype.h"

bool
sign_operator (int c);

size_t
sign_count (const tib_Expression *expr);

bool
contains_i (const tib_Expression *expr);

TIB *
tib_eval (const tib_Expression *expr);

int
tib_eval_surrounded (const tib_Expression *expr);

bool
tib_eval_isnum (const tib_Expression *expr);

bool
tib_eval_isstr (const tib_Expression *expr);

bool
tib_eval_islist (const tib_Expression *expr);

bool
tib_eval_ismatrix (const tib_Expression *expr);

int
tib_eval_close_parens (tib_Expression *expr);

int
tib_eval_parse_commas (const tib_Expression *expr, tib_Expression ***out,
		       size_t *out_len);

#endif
