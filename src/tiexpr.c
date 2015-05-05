/*
 *  libtibasic - Read, write, and evaluate TI BASIC programs
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

#include "tiexpr.h"
#include "tibasic.h"

tibasic_Expression *
tibasic_new_Expression ()
{
  tibasic_Expression *out = malloc (sizeof (tibasic_Expression));
  if (NULL == out)
    return NULL;

  out->len = 0;
  out->refs = 1;
  out->value = NULL;

  return out;
}

void
tibasic_Expression_incref (tibasic_Expression *expr)
{
  ++expr->refs;
}

void
tibasic_Expression_decref (tibasic_Expression *expr)
{
  if (0 == --expr->refs)
    {
      if (NULL != expr->value)
	free (expr->value);

      free (expr);
    }
}

int
tibasic_Expression_set (tibasic_Expression *expr, char *s)
{
  int *temp = expr->value;
  size_t len = strlen (s);

  expr->value = malloc (len * sizeof (int));
  if (NULL == expr->value)
    {
      expr->value = temp;
      return TIBASIC_EALLOC;
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
tibasic_Expression_clear (tibasic_Expression *expr)
{
  free (expr->value);
  expr->value = NULL;
  expr->len = 0;
}

char *
tibasic_Expression_as_str (tibasic_Expression *expr)
{
  if (NULL == expr->value)
    return NULL;

  char *s = malloc ((expr->len + 1) * sizeof (char));
  if (NULL == s)
    return NULL;

  size_t i;
  for (i = 0; i < expr->len; ++i)
    s[i] = expr->value[i];

  s[expr->len] = '\0';

  return s;
}

int
tibasic_Expression_remove (tibasic_Expression *expr, size_t i)
{
  if (i > expr->len)
    return TIBASIC_EINDEX;

  --expr->len;

  for (; i < expr->len; ++i)
    expr->value[i] = expr->value[i+1];

  return 0;
}

int
tibasic_Expression_insert (tibasic_Expression *expr, size_t i, int c)
{
  ++expr->len;

  if (i > expr->len)
    {
      --expr->len;
      return TIBASIC_EINDEX;
    }

  int *temp = expr->value;

  expr->value = malloc (expr->len * sizeof (int));
  if (NULL == expr->value)
    {
      expr->value = temp;
      return TIBASIC_EALLOC;
    }

  size_t j;
  for (j = 0; j < i; ++j)
    expr->value[j] = temp[j];

  expr->value[i] = c;

  for (j = i+1; j < expr->len; ++j)
    expr->value[j] = temp[j];

  free (temp);

  return 0;
}

int
tibasic_Expression_substring (tibasic_Expression *in, tibasic_Expression **out,
			      size_t beg, size_t end)
{
  if (beg > in->len || end > in->len || beg > end)
    return TIBASIC_EINDEX;

  char *instr = tibasic_Expression_as_str (in);
  if (NULL == instr)
    return TIBASIC_EALLOC;

  char *outstr = malloc ((end - (beg-1)) * sizeof (char));
  if (NULL == outstr)
    {
      free (instr);
      return TIBASIC_EALLOC;
    }

  for (; beg <= end; ++beg)
    outstr[beg] = instr[beg];
  free (instr);

  *out = tibasic_new_Expression ();
  if (NULL == *out)
    {
      free (outstr);
      return TIBASIC_EALLOC;
    }

  int rc = tibasic_Expression_set (*out, outstr);
  free (outstr);

  if (rc)
    tibasic_Expression_decref (*out);

  return rc;
}

int
tibasic_Expression_cat (tibasic_Expression *dest, tibasic_Expression *src)
{
  char *orig = tibasic_Expression_as_str (dest);
  if (NULL == orig)
    return TIBASIC_EALLOC;

  char *add = tibasic_Expression_as_str (src);
  if (NULL == add)
    {
      free (orig);
      return TIBASIC_EALLOC;
    }

  size_t len = tibasic_Expression_len (dest) + tibasic_Expression_len (src)
    + 1;

  char *new = malloc (len * sizeof (char));
  if (NULL == new)
    {
      free (orig);
      free (add);
      return TIBASIC_EALLOC;
    }

  sprintf (new, "%s%s", orig, add);
  free (orig);
  free (add);

  int rc = tibasic_Expression_set (dest, new);

  return rc;
}

int
tibasic_Expression_get_at (tibasic_Expression *expr, size_t i)
{
  return expr->value[i];
}

size_t
tibasic_Expression_len (tibasic_Expression *expr)
{
  return expr->len;
}
