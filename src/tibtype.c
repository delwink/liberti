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
#include <gsl/gsl_complex_math.h>
#include <math.h>

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
  TIB *temp;

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
      temp = tib_empty ();
      if (NULL == temp)
	return NULL;

      temp->type = TIB_TYPE_LIST;

      temp->value.list = gsl_vector_complex_alloc (t->value.list->size);
      if (!temp->value.list)
	{
	  tib_errno = TIB_EALLOC;
	  tib_decref (temp);
	  return NULL;
	}

      tib_errno = gsl_vector_complex_memcpy (temp->value.list, t->value.list);
      if (tib_errno)
	{
	  tib_decref (temp);
	  return NULL;
	}

      return temp;

    case TIB_TYPE_MATRIX:
      temp = tib_empty ();
      if (NULL == temp)
	return NULL;

      temp->type = TIB_TYPE_MATRIX;

      temp->value.matrix = gsl_matrix_complex_alloc (t->value.matrix->size1,
						     t->value.matrix->size2);
      if (!temp->value.matrix)
	{
	  tib_errno = TIB_EALLOC;
	  tib_decref (temp);
	  return NULL;
	}

      tib_errno = gsl_matrix_complex_memcpy (temp->value.matrix,
					     t->value.matrix);
      if (tib_errno)
	{
	  tib_decref (temp);
	  return NULL;
	}

      return temp;

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

const char *
tib_str_value (const TIB *t)
{
  if (t->type == TIB_TYPE_STRING)
    return t->value.string;

  tib_errno = TIB_ETYPE;
  return NULL;
}

const gsl_vector_complex *
tib_list_value (const TIB *t)
{
  if (t->type == TIB_TYPE_LIST)
    return t->value.list;

  tib_errno = TIB_ETYPE;
  return NULL;
}

const gsl_matrix_complex *
tib_matrix_value (const TIB *t)
{
  if (t->type == TIB_TYPE_MATRIX)
    return t->value.matrix;

  tib_errno = TIB_ETYPE;
  return NULL;
}

TIB *
tib_add (const TIB *t1, const TIB *t2)
{
  if (t1->type != t2->type
      && !(TIB_TYPE_COMPLEX == t1->type && TIB_TYPE_LIST == t2->type)
      && !(TIB_TYPE_LIST == t1->type && TIB_TYPE_COMPLEX == t2->type))
    {
      tib_errno = TIB_ETYPE;
      return NULL;
    }

  char *s;
  TIB *temp;
  size_t i;
  switch (t1->type)
    {
    case TIB_TYPE_COMPLEX:
      if (TIB_TYPE_COMPLEX == t2->type)
	{
	  temp = tib_copy (t1);
	  if (NULL == temp)
	    return NULL;

	  temp->value.number = gsl_complex_add (t1->value.number,
						t2->value.number);

	  return temp;
	}
      else
	{
	  temp = tib_copy (t2);
	  if (NULL == temp)
	    return NULL;

	  for (i = 0; i < temp->value.list->size; ++i)
	    {
	      gsl_complex a = gsl_vector_complex_get (t2->value.list, i);
	      gsl_complex sum = gsl_complex_add (a, t1->value.number);
	      gsl_vector_complex_set (temp->value.list, i, sum);
	    }

	  return temp;
	}

    case TIB_TYPE_STRING:
      s = malloc ((strlen (t1->value.string) + strlen (t2->value.string) + 1)
		  * sizeof (char));
      sprintf (s, "%s%s", t1->value.string, t2->value.string);
      temp = tib_new_str (s);
      free (s);
      return temp;

    case TIB_TYPE_LIST:
      if (TIB_TYPE_LIST == t2->type)
	{
	  temp = tib_copy (t1);
	  if (NULL == temp)
	    return NULL;

	  tib_errno = gsl_vector_complex_add (temp->value.list, t2->value.list);
	  if (tib_errno)
	    {
	      tib_decref (temp);
	      return NULL;
	    }

	  return temp;
	}
      else
	{
	  return tib_add (t2, t1);
	}

    case TIB_TYPE_MATRIX:
      temp = tib_copy (t1);
      if (NULL == temp)
	return NULL;

      tib_errno = gsl_matrix_complex_add (temp->value.matrix,
					  t2->value.matrix);
      if (tib_errno)
	{
	  tib_decref (temp);
	  return NULL;
	}

      return temp;

    default:
      tib_errno = TIB_ETYPE;
      return NULL;
    }
}

TIB *
tib_sub (const TIB *t1, const TIB *t2)
{
  if (t1->type != t2->type
      && !(TIB_TYPE_COMPLEX == t1->type && TIB_TYPE_LIST == t2->type)
      && !(TIB_TYPE_LIST == t1->type && TIB_TYPE_COMPLEX == t2->type))
    {
      tib_errno = TIB_ETYPE;
      return NULL;
    }

  TIB *temp;
  size_t i;
  switch (t1->type)
    {
    case TIB_TYPE_COMPLEX:
      if (TIB_TYPE_COMPLEX == t2->type)
	{
	  temp = tib_copy (t1);
	  if (NULL == temp)
	    return NULL;

	  temp->value.number = gsl_complex_sub (t1->value.number,
						t2->value.number);

	  return temp;
	}
      else
	{
	  temp = tib_copy (t2);
	  if (NULL == temp)
	    return NULL;

	  for (i = 0; i < temp->value.list->size; ++i)
	    {
	      gsl_complex a = gsl_vector_complex_get (t2->value.list, i);
	      gsl_complex diff = gsl_complex_sub (a, t1->value.number);
	      gsl_vector_complex_set (temp->value.list, i, diff);
	    }

	  return temp;
	}

    case TIB_TYPE_LIST:
      if (TIB_TYPE_LIST == t2->type)
	{
	  temp = tib_copy (t1);
	  if (NULL == temp)
	    return NULL;

	  tib_errno = gsl_vector_complex_sub (temp->value.list,
					      t2->value.list);
	  if (tib_errno)
	    {
	      tib_decref (temp);
	      return NULL;
	    }

	  return temp;
	}
      else
	{
	  temp = tib_copy (t2);
	  if (NULL == temp)
	    return NULL;

	  for (i = 0; i < temp->value.list->size; ++i)
	    {
	      gsl_complex a = gsl_vector_complex_get (t2->value.list, i);
	      gsl_complex diff = gsl_complex_sub (t1->value.number, a);
	      gsl_vector_complex_set (temp->value.list, i, diff);
	    }

	  return temp;
	}

    case TIB_TYPE_MATRIX:
      temp = tib_copy (t1);
      if (NULL == temp)
	return NULL;

      tib_errno = gsl_matrix_complex_sub (temp->value.matrix,
					  t2->value.matrix);
      if (tib_errno)
	{
	  tib_decref (temp);
	  return NULL;
	}

      return temp;

    default:
      tib_errno = TIB_ETYPE;
      return NULL;
    }
}

static int
matrix_mul (gsl_matrix_complex *out, gsl_matrix_complex *m1,
	    gsl_matrix_complex *m2)
{
  size_t i, j, k;
  const size_t DIM = m1->size2;

  gsl_complex *in = malloc (sizeof (gsl_complex[DIM]));
  if (NULL == in)
    return TIB_EALLOC;

  for (i = 0; i < m1->size1; ++i)
    for (j = 0; k < m2->size2; ++j)
      {
	for (k = 0; k < DIM; ++k)
	  in[k] = gsl_complex_mul (gsl_matrix_complex_get (m1, i, k),
				   gsl_matrix_complex_get (m2, k, j));

	gsl_complex sum;
	sum.dat[0] = 0;
	sum.dat[1] = 0;

	for (k = 0; k < DIM; ++k)
	  sum = gsl_complex_add (sum, in[k]);

	gsl_matrix_complex_set (out, i, j, sum);
      }

  free (in);
  return 0;
}

TIB *
tib_mul (const TIB *t1, const TIB *t2)
{
  if (t1->type != t2->type)
    {
      switch (t1->type)
	{
	case TIB_TYPE_COMPLEX:
	  if (TIB_TYPE_NONE == t2->type || TIB_TYPE_STRING == t2->type)
	    {
	      tib_errno = TIB_ETYPE;
	      return NULL;
	    }
	  break;

	case TIB_TYPE_LIST:
	case TIB_TYPE_MATRIX:
	  if (TIB_TYPE_COMPLEX != t2->type)
	    {
	      tib_errno = TIB_ETYPE;
	      return NULL;
	    }
	  break;

	default:
	  tib_errno = TIB_ETYPE;
	  return NULL;
	}
    }

  TIB *temp;
  size_t i, j;
  switch (t1->type)
    {
    case TIB_TYPE_COMPLEX:
      if (TIB_TYPE_COMPLEX == t2->type)
	{
	  temp = tib_copy (t1);
	  if (NULL == temp)
	    return NULL;

	  temp->value.number = gsl_complex_mul (t1->value.number,
						t2->value.number);

	  return temp;
	}
      else
	{
	  temp = tib_copy (t2);
	  if (NULL == temp)
	    return NULL;

	  gsl_complex a, product;
	  if (TIB_TYPE_LIST == t2->type)
	    {
	      for (i = 0; i < t2->value.list->size; ++i)
		{
		  a = gsl_vector_complex_get (t2->value.list, i);
		  product = gsl_complex_mul (t1->value.number, a);
		  gsl_vector_complex_set (temp->value.list, i, product);
		}
	    }
	  else /* must be matrix */
	    {
	      for (i = 0; i < t2->value.matrix->size1; ++i)
		for (j = 0; j < t2->value.matrix->size2; ++j)
		  {
		    a = gsl_matrix_complex_get (t2->value.matrix, i, j);
		    product = gsl_complex_mul (t1->value.number, a);
		    gsl_matrix_complex_set (temp->value.matrix, i, j, product);
		  }
	    }

	  return temp;
	}

    case TIB_TYPE_LIST:
      if (TIB_TYPE_LIST == t2->type)
	{
	  if (t1->value.list->size != t2->value.list->size)
	    {
	      tib_errno = TIB_EDIM;
	      return NULL;
	    }

	  temp = tib_copy (t1);
	  if (NULL == temp)
	    return NULL;

	  for (i = 0; i < temp->value.list->size; ++i)
	    {
	      gsl_complex a = gsl_vector_complex_get (t1->value.list, i);
	      gsl_complex b = gsl_vector_complex_get (t2->value.list, i);
	      gsl_complex product = gsl_complex_mul (a, b);
	      gsl_vector_complex_set (temp->value.list, i, product);
	    }

	  return temp;
	}
      else
	{
	  return tib_mul (t2, t1);
	}

    case TIB_TYPE_MATRIX:
      if (TIB_TYPE_MATRIX == t2->type)
	{
	  if (t1->value.matrix->size2 != t2->value.matrix->size1)
	    {
	      tib_errno = TIB_EDIM;
	      return NULL;
	    }

	  temp = tib_new_matrix (NULL, t1->value.matrix->size1,
				 t2->value.matrix->size2);
	  if (NULL == temp)
	    return NULL;

	  tib_errno = matrix_mul (temp->value.matrix, t1->value.matrix,
				  t2->value.matrix);
	  if (tib_errno)
	    {
	      tib_decref (temp);
	      return NULL;
	    }

	  return temp;
	}
      else
	{
	  return tib_mul (t2, t1);
	}

    default:
      tib_errno = TIB_ETYPE;
      return NULL;
    }
}

static bool
less_than_0 (gsl_complex x)
{
  return gsl_complex_abs (x) < 0;
}

static TIB *
inverse (TIB *t)
{
  /* TODO: convert t to the inverse of t */
  return t;
}

TIB *
tib_pow (const TIB *t, gsl_complex exp)
{
  TIB *temp = tib_copy (t);
  if (NULL == temp)
    return NULL;

  if (less_than_0 (exp))
    {
      temp = inverse (temp);
      if (NULL == temp)
	{
	  tib_decref (temp);
	  return NULL;
	}
    }

  double abs_exp = gsl_complex_abs (exp);

  switch (t->type)
    {
    case TIB_TYPE_COMPLEX:
      temp->value.number = gsl_complex_pow_real (t->value.number, abs_exp);
      return temp;

    default:
      tib_errno = TIB_ETYPE;
      return NULL;
    }
}
