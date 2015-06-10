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

#ifndef DELWINK_TIB_TIBEXPR_H
#define DELWINK_TIB_TIBEXPR_H

#include <stdlib.h>
#include <gsl/gsl_complex.h>

#define tib_foreachexpr(E,I) for (I = 0; I < tib_Expression_len (E); ++I)

typedef struct
{
  size_t len;
  int *value;
  size_t refs;
} tib_Expression;

tib_Expression *
tib_new_Expression (void);

tib_Expression *
tib_copy_Expression (const tib_Expression *expr);

void
tib_Expression_incref (tib_Expression *expr);

void
tib_Expression_decref (tib_Expression *expr);

int
tib_Expression_set (tib_Expression *expr, char *s);

void
tib_Expression_clear (tib_Expression *expr);

char *
tib_Expression_as_str (const tib_Expression *expr);

int
tib_Expression_as_num (const tib_Expression *expr, gsl_complex *out);

int
tib_Expression_remove (tib_Expression *expr, size_t i);

int
tib_Expression_insert (tib_Expression *expr, size_t i, int c);

int
tib_Expression_push (tib_Expression *expr, int c);

int
tib_Expression_substring (const tib_Expression *in, tib_Expression **out,
			  size_t beg, size_t end);

int
tib_Expression_cat (tib_Expression *dest, tib_Expression *src);

int
tib_Expression_ref (const tib_Expression *expr, size_t i);

size_t
tib_Expression_len (const tib_Expression *expr);

#endif
