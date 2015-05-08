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

#ifndef DELWINK_TIB_TIBTYPE_H
#define DELWINK_TIB_TIBTYPE_H

#include <stdlib.h>
#include <stdint.h>

#define TIB_TYPE_REAL      1

#define TIB_TYPE_IMAGINARY 2

#define TIB_TYPE_STRING    3

#define TIB_TYPE_LIST      4

#define TIB_TYPE_MATRIX    5

#define TIB_TYPE_COMPLEX   6

struct complex
{
  double real;
  double imaginary;
};

struct list
{
  void *values;
  size_t len;
};

struct matrix
{
  void **values;
  size_t width;
  size_t height;
};

union variant
{
  struct complex number;
  char *string;
  struct list list;
  struct matrix matrix;
};

typedef struct
{
  int8_t type;
  union variant value;
  size_t refs;
} TIB;

TIB *
tib_empty (void);

TIB *
tib_copy (const TIB *t);

void
tib_incref (TIB *t);

void
tib_decref (TIB *t);

TIB *
tib_new_real (double value);

TIB *
tib_new_imaginary (double value);

TIB *
tib_new_str (const char *value);

TIB *
tib_new_list (const TIB **value, size_t len);

TIB *
tib_new_matrix (const TIB ***value, size_t w, size_t h);

void
tib_set_type (TIB *t, int8_t type);

int8_t
tib_type (const TIB *t);

bool
tib_isreal (const TIB *t);

bool
tib_isimaginary (const TIB *t);

bool
tib_iscomplex (const TIB *t);

bool
tib_isnum (const TIB *t);

bool
tib_isstr (const TIB *t);

bool
tib_islist (const TIB *t);

bool
tib_ismatrix (const TIB *t);

int
tib_real_value (const TIB *t, double *out);

int
tib_imaginary_value (const TIB *t, double *out);

int
tib_complex_value (const TIB *t, double *real_out, double *imaginary_out);

#endif
