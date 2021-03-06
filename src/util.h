/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016-2017 Delwink, LLC
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

#ifndef DELWINK_LIBERTI_UTIL_H
#define DELWINK_LIBERTI_UTIL_H

#include "tibexpr.h"

int
load_expr(struct tib_expr *dest, const char *src);

int
load_expr_num(struct tib_expr *dest, const char *src);

const char *
display_special_char(int c);

char *
get_expr_display_str(const struct tib_expr *expr);

int
max(int x, int y);

int
min(int x, int y);

#endif
