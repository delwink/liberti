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

#ifndef DELWINK_TIBASIC_EVAL_H
#define DELWINK_TIBASIC_EVAL_H

#include <stdbool.h>

#include "tiexpr.h"
#include "titype.h"

TI *
eval (tibasic_Expression expr);

bool
tibasic_eval_surrounded (tibasic_Expression *expr);

bool
tibasic_eval_surrounded_fct (tibasic_Expression *expr, int fct);

bool
tibasic_eval_isnum (tibasic_Expression *expr);

bool
tibasic_eval_isstr (tibasic_Expression *expr);

bool
tibasic_eval_islist (tibasic_Expression *expr);

bool
tibasic_eval_ismatrix (tibasic_Expression *expr);

int
tibasic_eval_find_outside_parens (tibasic_Expression *expr, int c);

void
tibasic_eval_fix_parens (tibasic_Expression **out);

tibasic_Expression *
tibasic_eval_parse_commas (tibasic_Expression *expr);

#endif
