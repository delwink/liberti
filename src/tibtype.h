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
#include <stdbool.h>

enum tib_types
  {
    TIB_TYPE_NONE=0,
    TIB_TYPE_REAL,
    TIB_TYPE_IMAGINARY,
    TIB_TYPE_COMPLEX,
    TIB_TYPE_STRING,
    TIB_TYPE_LIST,
    TIB_TYPE_MATRIX
  };

extern int tib_errno;

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
  void *values;
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
tib_new_complex (double real, double imaginary);

TIB *
tib_new_str (const char *value);

TIB *
tib_new_list (const TIB *value, size_t len);

TIB *
tib_new_matrix (const TIB **value, size_t w, size_t h);

int8_t
tib_type (const TIB *t);

double
tib_real_value (const TIB *t);

double
tib_imaginary_value (const TIB *t);

struct complex
tib_complex_value (const TIB *t);

char *
tib_str_value (const TIB *t);

struct list
tib_list_value (const TIB *t);

struct matrix
tib_matrix_value (const TIB *t);

TIB *
tib_add (const TIB *t1, const TIB *t2);

TIB *
tib_subtract (const TIB *t1, const TIB *t2);

TIB *
tib_multiply (const TIB *t1, const TIB *t2);

TIB *
tib_divide (const TIB *t1, const TIB *t2);

#endif
