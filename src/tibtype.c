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

    case TIB_TYPE_COMPLEX:
      return tib_new_complex (GSL_REAL (t->value.number),
			      GSL_IMAG (t->value.number));

    case TIB_TYPE_STRING:
      return tib_new_str (t->value.string);

    case TIB_TYPE_LIST:
      return tib_new_list ((gsl_complex *) t->value.list->data,
			   t->value.list->size);

    case TIB_TYPE_MATRIX:
      return tib_new_matrix ((const gsl_complex **) t->value.matrix->data,
			     t->value.matrix->size1, t->value.matrix->size2);

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
	  gsl_vector_complex_free (t->value.list);
	  break;

	case TIB_TYPE_MATRIX:
	  gsl_matrix_complex_free (t->value.matrix);
	  break;

	case TIB_TYPE_STRING:
	  free (t->value.string);
	  break;
	}

      free (t);
    }
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
  GSL_SET_COMPLEX (&out->value.number, real, imaginary);

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
tib_new_list (const gsl_complex *value, size_t len)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_LIST;
  out->refs = 1;
  out->value.list = gsl_vector_complex_alloc (len);
  if (!out->value.list)
    {
      tib_errno = TIB_EALLOC;
      free (out);
      return NULL;
    }

  size_t i;
  if (value != NULL)
    for (i = 0; i < len; ++i)
      gsl_vector_complex_set (out->value.list, i, value[i]);

  return out;
}

TIB *
tib_new_matrix (const gsl_complex **value, size_t w, size_t h)
{
  TIB *out = malloc (sizeof (TIB));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->type = TIB_TYPE_MATRIX;
  out->refs = 1;
  out->value.matrix = gsl_matrix_complex_alloc (w, h);
  if (!out->value.matrix)
    {
      tib_errno = TIB_EALLOC;
      free (out);
      return NULL;
    }

  size_t i, j;
  if (value != NULL)
    for (i = 0; i < w; ++i)
      for (j = 0; j < h; ++j)
	gsl_matrix_complex_set (out->value.matrix, i, j, value[i][j]);

  return out;
}

int8_t
tib_type (const TIB *t)
{
  return t->type;
}

gsl_complex
tib_complex_value (const TIB *t)
{
  if (t->type == TIB_TYPE_COMPLEX)
    return t->value.number;

  tib_errno = TIB_ETYPE;
  return (gsl_complex) {.dat = {0, 0}};
}

char *
tib_str_value (const TIB *t)
{
  if (t->type == TIB_TYPE_STRING)
    return t->value.string;

  tib_errno = TIB_ETYPE;
  return NULL;
}

gsl_vector_complex *
tib_list_value (const TIB *t)
{
  if (t->type == TIB_TYPE_LIST)
    return t->value.list;

  tib_errno = TIB_ETYPE;
  return NULL;
}

gsl_matrix_complex *
tib_matrix_value (const TIB *t)
{
  if (t->type == TIB_TYPE_MATRIX)
    return t->value.matrix;

  tib_errno = TIB_ETYPE;
  return NULL;
}

static bool
number_type (const TIB *t)
{
  return TIB_TYPE_COMPLEX == t->type;
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
  TIB *temp;
  int rc;
  switch (t1->type)
    {
    case TIB_TYPE_COMPLEX:
      return tib_new_complex ((GSL_REAL (t1->value.number)
			       + GSL_REAL (t2->value.number)),
			      (GSL_IMAG (t1->value.number)
			       + GSL_IMAG (t2->value.number)));

    case TIB_TYPE_STRING:
      s = malloc ((strlen (t1->value.string) + strlen (t2->value.string) + 1)
		  * sizeof (char));
      sprintf (s, "%s%s", t1->value.string, t2->value.string);
      temp = tib_new_str (s);
      free (s);
      return temp;

    case TIB_TYPE_LIST:
      temp = tib_copy (t1);
      if (NULL == temp)
	return NULL;

      rc = gsl_vector_complex_add (temp->value.list, t2->value.list);
      if (rc)
	{
	  tib_errno = rc;
	  tib_decref (temp);
	  return NULL;
	}

      return temp;

    case TIB_TYPE_MATRIX:
      temp = tib_copy (t1);
      if (NULL == temp)
	return NULL;

      rc = gsl_matrix_complex_add (temp->value.matrix, t2->value.matrix);
      if (rc)
	{
	  tib_errno = rc;
	  tib_decref (temp);
	  return NULL;
	}

      return temp;

    default:
      tib_errno = TIB_ETYPE;
      return NULL;
    }
}
