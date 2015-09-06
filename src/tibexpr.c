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

#include <stdio.h>
#include <string.h>

#include "tiberr.h"
#include "tibexpr.h"
#include "tibchar.h"
#include "tibeval.h"

tib_Expression *
tib_new_Expression ()
{
  tib_Expression *out = malloc (sizeof (tib_Expression));
  if (NULL == out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  out->len = 0;
  out->refs = 1;
  out->value = NULL;

  return out;
}

tib_Expression *
tib_copy_Expression (const tib_Expression *expr)
{
  tib_Expression *out = tib_new_Expression ();
  if (NULL == out)
    return NULL;

  size_t i;
  tib_foreachexpr (expr, i)
    {
      tib_errno = tib_Expression_push (out, tib_Expression_ref (expr, i));
      if (tib_errno)
	break;
    }

  if (tib_errno)
    {
      tib_Expression_decref (out);
      return NULL;
    }

  return out;
}

void
tib_Expression_incref (tib_Expression *expr)
{
  ++expr->refs;
}

void
tib_Expression_decref (tib_Expression *expr)
{
  if (0 == --expr->refs)
    {
      if (NULL != expr->value)
	free (expr->value);

      free (expr);
    }
}

int
tib_Expression_set (tib_Expression *expr, char *s)
{
  int *temp = expr->value;
  size_t len = strlen (s);

  expr->value = malloc (len * sizeof (int));
  if (NULL == expr->value)
    {
      expr->value = temp;
      return TIB_EALLOC;
    }

  size_t i;
  for (i = 0; i < len; ++i)
    expr->value[i] = s[i];

  expr->len = len;

  if (NULL == temp)
    free (temp);

  return 0;
}

void
tib_Expression_clear (tib_Expression *expr)
{
  free (expr->value);
  expr->value = NULL;
  expr->len = 0;
}

char *
tib_Expression_as_str (const tib_Expression *expr)
{
  size_t i, bump = 0, len = 1;
  char *s;
  const char *t;

  if (NULL == expr->value)
    {
      tib_errno = TIB_ENULLPTR;
      return NULL;
    }

  tib_foreachexpr (expr, i)
    {
      t = tib_special_char_text (expr->value[i]);
      if (t)
	len += strlen (t);
      else
	++len;
    }

  s = malloc (len * sizeof (char));
  if (NULL == s)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  tib_foreachexpr (expr, i)
    {
      t = tib_special_char_text (expr->value[i]);
      if (t)
	{
	  s[i+bump] = '\0';
	  strcat (s, t);
	  bump += strlen (t) - 1;
	}
      else
	{
	  s[i+bump] = expr->value[i];
	}
    }

  s[i+bump] = '\0';

  return s;
}

int
tib_Expression_as_num (const tib_Expression *expr, gsl_complex *out)
{
  if (!tib_eval_isnum (expr))
    return TIB_ESYNTAX;

  size_t i, numop = sign_count (expr);

  char *s = tib_Expression_as_str (expr);
  if (NULL == s)
    return TIB_EALLOC;

  char *i_start = NULL;
  if (contains_i (expr))
    for (i = 0; i < strlen(s); ++i)
      {
	if ('i' == s[i])
	  {
	    i_start = s;
	    break;
	  }
	else if (sign_operator (s[i]) && --numop == 0)
	  {
	    i_start = &(s[i]);
	    break;
	  }
      }

  GSL_SET_REAL (out, s == i_start ? 0 : strtod (s, NULL));

  if (i_start)
    {
      if (sign_operator (i_start[0]) && 'i' == i_start[1])
	i_start[1] = '1';
      else if ('i' == i_start[0])
	i_start[0] = '1';
    }

  GSL_SET_IMAG (out, NULL == i_start ? 0 : strtod (i_start, NULL));

  free (s);
  return 0;
}

int
tib_Expression_remove (tib_Expression *expr, size_t i)
{
  if (i > expr->len)
    return TIB_EINDEX;

  --expr->len;

  for (; i < expr->len; ++i)
    expr->value[i] = expr->value[i+1];

  return 0;
}

int
tib_Expression_insert (tib_Expression *expr, size_t i, int c)
{
  ++expr->len;

  if (i > expr->len)
    {
      --expr->len;
      return TIB_EINDEX;
    }

  int *temp = expr->value;

  expr->value = malloc (expr->len * sizeof (int));
  if (NULL == expr->value)
    {
      free (temp);
      --expr->len;
      return TIB_EALLOC;
    }

  size_t j;
  for (j = 0; j < i; ++j)
    expr->value[j] = temp[j];

  expr->value[i] = c;

  for (j = i+1; j < expr->len; ++j)
    expr->value[j] = temp[j-1];

  free (temp);

  return 0;
}

int
tib_Expression_push (tib_Expression *expr, int c)
{
  return tib_Expression_insert (expr, tib_Expression_len (expr), c);
}

tib_Expression *
tib_Expression_substring (const tib_Expression *in, size_t beg, size_t end)
{
  if (beg > in->len || end > in->len || beg > end)
    {
      tib_errno = TIB_EINDEX;
      return NULL;
    }

  tib_Expression *out = tib_new_Expression ();
  if (NULL == out)
    return NULL;

  for (; beg <= end; ++beg)
    {
      tib_errno = tib_Expression_push (out, tib_Expression_ref (in, beg));
      if (tib_errno)
	break;
    }

  if (tib_errno)
    {
      tib_Expression_decref (out);
      return NULL;
    }

  return out;
}

int
tib_Expression_cat (tib_Expression *dest, const tib_Expression *src)
{
  int rc;
  size_t i, oldlen = tib_Expression_len (dest);

  tib_foreachexpr (src, i)
    {
      rc = tib_Expression_push (dest, tib_Expression_ref (src, i));
      if (rc)
	break;
    }

  if (rc)
    dest->len = oldlen;

  return rc;
}

int
tib_Expression_ref (const tib_Expression *expr, size_t i)
{
  return expr->value[i];
}

size_t
tib_Expression_len (const tib_Expression *expr)
{
  return expr->len;
}
