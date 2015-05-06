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

#include <stdint.h>

#define TIB_TYPE_REAL      1

#define TIB_TYPE_IMAGINARY 2

#define TIB_TYPE_STRING    3

#define TIB_TYPE_LIST      4

#define TIB_TYPE_MATRIX    5

union variant
{
  double real;
  double imaginary;
  char *string;
  char **list;
  char ***matrix;
};

typedef struct
{
  int8_t type;
  union variant value;
} TIB;

#endif
