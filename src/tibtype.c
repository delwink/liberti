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

#include <string.h>
#include <stdio.h>

#include "tib.h"
#include "tibtype.h"

int tib_errno = 0;

TIB *
tib_empty ()
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_NONE;
  out->refs = 1;

  return out;
}

TIB *
tib_copy (const TIB *t)
{
  switch (t->type)
    {
    case TIB_TYPE_NONE:
      return tib_empty ();

    case TIB_TYPE_REAL:
      return tib_new_real (t->value.number.real);

    case TIB_TYPE_IMAGINARY:
      return tib_new_imaginary (t->value.number.imaginary);

    case TIB_TYPE_COMPLEX:
      return tib_new_complex (t->value.number.real, t->value.number.imaginary);

    case TIB_TYPE_STRING:
      return tib_new_str (t->value.string);

    case TIB_TYPE_LIST:
      return tib_new_list (t->value.list.values, t->value.list.len);

    case TIB_TYPE_MATRIX:
      return tib_new_matrix ((const TIB **) t->value.matrix.values,
			     t->value.matrix.width, t->value.matrix.height);

    default:
      return NULL;
    }
}

void
tib_incref (TIB *t)
{
  ++t->refs;
}

void
tib_decref (TIB *t)
{
  if (--t->refs == 0)
    {
      switch (t->type)
	{
	case TIB_TYPE_LIST:
	  free (t->value.list.values);
	  break;

	case TIB_TYPE_MATRIX:
	  free (t->value.matrix.values);
	  break;

	case TIB_TYPE_STRING:
	  free (t->value.string);
	  break;
	}

      free (t);
    }
}

TIB *
tib_new_real (double value)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_REAL;
  out->refs = 1;
  out->value.number.real = value;

  return out;
}

TIB *
tib_new_imaginary (double value)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_IMAGINARY;
  out->refs = 1;
  out->value.number.imaginary = value;

  return out;
}

TIB *
tib_new_complex (double real, double imaginary)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_COMPLEX;
  out->refs = 1;
  out->value.number.real = real;
  out->value.number.imaginary = imaginary;

  return out;
}

TIB *
tib_new_str (const char *value)
{
  if (NULL == value)
    return NULL;

  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_STRING;
  out->refs = 1;
  out->value.string = malloc ((strlen (value) + 1) * sizeof (char));
  if (NULL == out->value.string)
    {
      tib_errno = TIB_EALLOC;
      free (out);
      return NULL;
    }

  strcpy (out->value.string, value);
  return out;
}

TIB *
tib_new_list (const TIB *value, size_t len)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_LIST;
  out->refs = 1;
  out->value.list.values = malloc (sizeof (TIB[len]));
  if (NULL == out->value.list.values)
    {
      tib_errno = TIB_EALLOC;
      free (out);
      return NULL;
    }

  size_t i;
  if (value != NULL)
    for (i = 0; i < len; ++i)
      ((TIB *) out->value.list.values)[i] = value[i];

  return out;
}

TIB *
tib_new_matrix (const TIB **value, size_t w, size_t h)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_MATRIX;
  out->refs = 1;
  out->value.matrix.values = malloc (sizeof (TIB[w][h]));
  if (NULL == out->value.matrix.values)
    {
      tib_errno = TIB_EALLOC;
      free (out);
      return NULL;
    }

  size_t i, j;
  if (value != NULL)
    for (i = 0; i < w; ++i)
      for (j = 0; j < h; ++j)
	((TIB **) out->value.matrix.values)[i][j] = value[i][j];

  return out;
}

int8_t
tib_type (const TIB *t)
{
  return t->type;
}

double
tib_real_value (const TIB *t)
{
  if (t->type == TIB_TYPE_REAL || t->type == TIB_TYPE_COMPLEX)
    return t->value.number.real;

  tib_errno = TIB_ETYPE;
  return 0;
}

double
tib_imaginary_value (const TIB *t)
{
  if (t->type == TIB_TYPE_IMAGINARY || t->type == TIB_TYPE_COMPLEX)
    return t->value.number.imaginary;

  tib_errno = TIB_ETYPE;
  return 0;
}

struct complex
tib_complex_value (const TIB *t)
{
  if (t->type == TIB_TYPE_COMPLEX)
    return t->value.number;

  tib_errno = TIB_ETYPE;
  return (struct complex) {0, 0};
}

char *
tib_str_value (const TIB *t)
{
  if (t->type == TIB_TYPE_STRING)
    return t->value.string;

  tib_errno = TIB_ETYPE;
  return NULL;
}

struct list
tib_list_value (const TIB *t)
{
  if (t->type == TIB_TYPE_LIST)
    return t->value.list;

  tib_errno = TIB_ETYPE;
  return (struct list) {NULL, 0};
}

struct matrix
tib_matrix_value (const TIB *t)
{
  if (t->type == TIB_TYPE_MATRIX)
    return t->value.matrix;

  tib_errno = TIB_ETYPE;
  return (struct matrix) {NULL, 0, 0};
}

static bool
number_type (const TIB *t)
{
  return (t->type == TIB_TYPE_REAL || t->type == TIB_TYPE_IMAGINARY
	  || t->type == TIB_TYPE_COMPLEX);
}

TIB *
tib_add (const TIB *t1, const TIB *t2)
{
  if (t1->type != t2->type && !(number_type (t1) && number_type (t2)))
    {
      tib_errno = TIB_ETYPE;
      return NULL;
    }

  char *s;
  TIB *temp[2];
  size_t i, j;
  switch (t1->type)
    {
    case TIB_TYPE_REAL:
    case TIB_TYPE_IMAGINARY:
    case TIB_TYPE_COMPLEX:
      return tib_new_complex ((t1->value.number.real + t2->value.number.real),
			      (t1->value.number.imaginary
			       + t2->value.number.imaginary));

    case TIB_TYPE_STRING:
      s = malloc ((strlen (t1->value.string) + strlen (t2->value.string) + 1)
		  * sizeof (char));
      sprintf (s, "%s%s", t1->value.string, t2->value.string);
      temp[0] = tib_new_str (s);
      free (s);
      return temp[0];

    case TIB_TYPE_LIST:
      if (t1->value.list.len != t2->value.list.len)
	{
	  tib_errno = TIB_EDIM;
	  return NULL;
	}

      temp[0] = tib_new_list (NULL, t1->value.list.len);
      if (NULL == temp[0])
	return NULL;

      for (i = 0; i < t1->value.list.len; ++i)
	{
	  temp[1] = tib_add (&((TIB *) t1->value.list.values)[i],
			     &((TIB *) t2->value.list.values)[i]);
	  if (NULL == temp[1])
	    break;

	  ((TIB *) temp[0]->value.list.values)[i] = *temp[1];
	  tib_decref (temp[1]);
	}

      if (tib_errno)
	{
	  tib_decref (temp[0]);
	  return NULL;
	}

      return temp[0];

    case TIB_TYPE_MATRIX:
      if (t1->value.matrix.height != t2->value.matrix.height
	  || t1->value.matrix.width != t2->value.matrix.width)
	{
	  tib_errno = TIB_EDIM;
	  return NULL;
	}

      temp[0] = tib_new_matrix (NULL, t1->value.matrix.width,
				t1->value.matrix.height);
      if (NULL == temp[0])
	return NULL;

      for (i = 0; i < t1->value.matrix.width; ++i)
	for (j = 0; j < t1->value.matrix.height; ++j)
	  {
	    temp[1] = tib_add (&((TIB **) t1->value.matrix.values)[i][j],
			       &((TIB **) t2->value.matrix.values)[i][j]);
	    if (NULL == temp[1])
	      break;

	    ((TIB **) temp[0]->value.matrix.values)[i][j] = *temp[1];
	    tib_decref (temp[1]);
	  }

      if (tib_errno)
	{
	  tib_decref (temp[0]);
	  return NULL;
	}

      return temp[0];

    default:
      tib_errno = TIB_ETYPE;
      return NULL;
    }
}
