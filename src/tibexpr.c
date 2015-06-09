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

tib_Expression *
tib_new_Expression ()
{
  tib_Expression *out = malloc (sizeof (tib_Expression));
  if (NULL == out)
    return NULL;

  out->len = 0;
  out->refs = 1;
  out->value = NULL;

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
    return NULL;

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
    return NULL;

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
      expr->value = temp;
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

int
tib_Expression_substring (const tib_Expression *in, tib_Expression **out,
			  size_t beg, size_t end)
{
  if (beg > in->len || end > in->len || beg > end)
    return TIB_EINDEX;

  char *instr = tib_Expression_as_str (in);
  if (NULL == instr)
    return TIB_EALLOC;

  char *outstr = malloc ((end - (beg-1)) * sizeof (char));
  if (NULL == outstr)
    {
      free (instr);
      return TIB_EALLOC;
    }

  for (; beg <= end; ++beg)
    outstr[beg] = instr[beg];
  free (instr);

  *out = tib_new_Expression ();
  if (NULL == *out)
    {
      free (outstr);
      return TIB_EALLOC;
    }

  int rc = tib_Expression_set (*out, outstr);
  free (outstr);

  if (rc)
    tib_Expression_decref (*out);

  return rc;
}

int
tib_Expression_cat (tib_Expression *dest, tib_Expression *src)
{
  char *orig = tib_Expression_as_str (dest);
  if (NULL == orig)
    return TIB_EALLOC;

  char *add = tib_Expression_as_str (src);
  if (NULL == add)
    {
      free (orig);
      return TIB_EALLOC;
    }

  size_t len = tib_Expression_len (dest) + tib_Expression_len (src)
    + 1;

  char *new = malloc (len * sizeof (char));
  if (NULL == new)
    {
      free (orig);
      free (add);
      return TIB_EALLOC;
    }

  sprintf (new, "%s%s", orig, add);
  free (orig);
  free (add);

  int rc = tib_Expression_set (dest, new);

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
