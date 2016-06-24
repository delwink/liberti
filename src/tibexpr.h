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

#ifndef DELWINK_TIB_TIBEXPR_H
#define DELWINK_TIB_TIBEXPR_H

#include <gsl/gsl_complex.h>

#define tib_expr_foreach(E,I) for ((I) = 0; (I) < (E)->len; ++(I))

struct tib_expr
{
  int *data;
  unsigned int len;
  unsigned int bufsize;
};

int
tib_expr_init (struct tib_expr *self);

void
tib_expr_free_data (struct tib_expr *self);

int
tib_exprcpy (struct tib_expr *dest, const struct tib_expr *src);

int
tib_exprcat (struct tib_expr *dest, const struct tib_expr *src);

char *
tib_expr_tostr (const struct tib_expr *self);

int
tib_expr_parse_complex (const struct tib_expr *self, gsl_complex *out);

int
tib_expr_delete (struct tib_expr *self, unsigned int i);

int
tib_expr_insert (struct tib_expr *self, unsigned int i, int c);

int
tib_expr_push (struct tib_expr *self, int c);

int
tib_subexpr (struct tib_expr *dest, const struct tib_expr *src,
	     unsigned int beg, unsigned int end);

#endif
