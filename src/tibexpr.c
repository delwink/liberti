/*
 *  libtib - Read, write, and evaluate TI BASIC programs
 *  Copyright (C) 2015-2016 Delwink, LLC
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

#define BUFFER_BLOCK_SIZE 16

int
tib_expr_init (struct tib_expr *self)
{
  self->data = malloc (BUFFER_BLOCK_SIZE * sizeof (int));
  if (!self->data)
    return TIB_EALLOC;

  self->bufsize = BUFFER_BLOCK_SIZE;
  self->len = 0;

  return 0;
}

void
tib_expr_destroy (struct tib_expr *self)
{
  if (self->bufsize)
    {
      free (self->data);

      self->data = NULL;
      self->bufsize = 0;
      self->len = 0;
    }
}

int
tib_exprcpy (struct tib_expr *dest, const struct tib_expr *src)
{
  dest->len = 0;
  return tib_exprcat (dest, src);
}

int
tib_exprcat (struct tib_expr *dest, const struct tib_expr *src)
{
  int rc;

  if (!dest->bufsize)
    {
      rc = tib_expr_init (dest);
      if (rc)
        return rc;
    }

  unsigned int i;
  tib_expr_foreach (src, i)
    {
      rc = tib_expr_push (dest, src->data[i]);
      if (rc)
        {
          tib_expr_destroy (dest);
          return rc;
        }
    }

  return 0;
}

char *
tib_expr_tostr (const struct tib_expr *self)
{
  if (!self->data)
    {
      tib_errno = TIB_ENULLPTR;
      return NULL;
    }

  unsigned int i, len = 1;
  tib_expr_foreach (self, i)
    {
      const char *special = tib_special_char_text (self->data[i]);
      if (special)
        len += strlen (special);
      else
        ++len;
    }

  char *out = malloc (len * sizeof (char));
  if (!out)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  unsigned int bump = 0;
  tib_expr_foreach (self, i)
    {
      const char *special = tib_special_char_text (self->data[i]);
      if (special)
        {
          out[i + bump] = '\0';
          strcat (out, special);
          bump += strlen (special) - 1;
        }
      else
        {
          out[i + bump] = self->data[i];
        }
    }

  out[i + bump] = '\0';
  return out;
}

int
tib_expr_parse_complex (const struct tib_expr *self, gsl_complex *out)
{
  if (!tib_eval_isnum (self))
    return TIB_ESYNTAX;

  const unsigned int len = self->len;
  char s[len + 1];

  for (unsigned int i = 0; i < len; ++i)
    s[i] = self->data[i];

  s[len] = '\0';

  char *i_start = NULL;
  if (contains_i (self))
    {
      unsigned int num_operators = sign_count (self);
      for (unsigned int i = 0; i < len; ++i)
        {
          int c = self->data[i];

          if ('i' == c)
            {
              i_start = s;
              break;
            }
          else if (sign_operator (c) && --num_operators == 0)
            {
              i_start = &s[i];
              break;
            }
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

  GSL_SET_IMAG (out, i_start ? strtod (i_start, NULL) : 0);

  return 0;
}

int
tib_expr_delete (struct tib_expr *self, unsigned int i)
{
  if (i > self->len)
    return TIB_EINDEX;

  --self->len;

  for (; i < self->len; ++i)
    self->data[i] = self->data[i + 1];

  return 0;
}

int
tib_expr_insert (struct tib_expr *self, unsigned int i, int c)
{
  if (i > ++self->len)
    {
      --self->len;
      return TIB_EINDEX;
    }

  if (!self->bufsize)
    {
      struct tib_expr temp = { .bufsize = 0 };
      int rc = tib_exprcpy (&temp, self);
      if (rc)
        return rc;

      *self = temp;
    }
  else if (self->len > self->bufsize)
    {
      int *old = self->data;
      self->bufsize *= 2;

      self->data = realloc (self->data, self->bufsize * sizeof (int));
      if (!self->data)
        {
          self->bufsize /= 2;
          --self->len;
          self->data = old;
          return TIB_EALLOC;
        }
    }

  for (unsigned int j = self->len - 1; j > i; --j)
    self->data[j] = self->data[j - 1];

  self->data[i] = c;
  return 0;
}

int
tib_expr_push (struct tib_expr *self, int c)
{
  return tib_expr_insert (self, self->len, c);
}

int
tib_subexpr (struct tib_expr *dest, const struct tib_expr *src,
             unsigned int beg, unsigned int end)
{
  if (end > src->len || end < beg)
    return TIB_EINDEX;

  dest->len = end - beg;
  dest->bufsize = 0;
  dest->data = &src->data[beg];

  return 0;
}
