/*
 *  libtib - Read, write, and evaluate TI BASIC programs
 *  Copyright (C) 2015-2017 Delwink, LLC
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

#ifndef DELWINK_TIB_TIBTYPE_H
#define DELWINK_TIB_TIBTYPE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <gsl/gsl_matrix_complex_double.h>
#include <gsl/gsl_complex.h>

#include "tibexpr.h"

enum tib_type
{
	TIB_TYPE_NONE = 0,
	TIB_TYPE_COMPLEX,
	TIB_TYPE_STRING,
	TIB_TYPE_LIST,
	TIB_TYPE_MATRIX
};

union variant
{
	gsl_complex number;
	char *string;
	gsl_vector_complex *list;
	gsl_matrix_complex *matrix;
};

typedef struct
{
	enum tib_type type;
	union variant value;
	size_t refs;
} TIB;

TIB *
tib_empty(void);

TIB *
tib_copy(const TIB *t);

void
tib_incref(TIB *t);

void
tib_decref(TIB *t);

TIB *
tib_new_complex(double real, double imaginary);

TIB *
tib_new_str(const char *value);

TIB *
tib_new_list(const gsl_complex *value, size_t len);

TIB *
tib_new_matrix(const gsl_complex **value, size_t w, size_t h);

enum tib_type
tib_type(const TIB *t);

gsl_complex
tib_complex_value(const TIB *t);

const char *
tib_str_value(const TIB *t);

const gsl_vector_complex *
tib_list_value(const TIB *t);

const gsl_matrix_complex *
tib_matrix_value(const TIB *t);

int
tib_toexpr(struct tib_expr *dest, const TIB *src);

TIB *
tib_add(const TIB *t1, const TIB *t2);

TIB *
tib_sub(const TIB *t1, const TIB *t2);

TIB *
tib_mul(const TIB *t1, const TIB *t2);

TIB *
tib_div(const TIB *t1, const TIB *t2);

TIB *
tib_pow(const TIB *t, const TIB *power);

TIB *
tib_root(const TIB *t, gsl_complex root);

TIB *
tib_factorial(const TIB *t);

TIB *
tib_log(const TIB *t);

TIB *
tib_toradians(const TIB *t);

#endif
