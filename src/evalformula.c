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

#include <ctype.h>

#include "tib.h"
#include "evalformula.h"
#include "tibchar.h"

static bool
is_left_paren (int c)
{
  return ('(' == c || TIB_CHAR_SIN == c || TIB_CHAR_RANDINT == c
	  || TIB_CHAR_NOT == c || TIB_CHAR_INT == c || TIB_CHAR_DIM == c
	  || TIB_CHAR_PIXEL_TEST == c);
}

bool
tib_eval_surrounded_function (const tib_Expression *expr, int function)
{
  int count = 0;
  size_t i, len = tib_Expression_len (expr);

  if (len > 2 && function == tib_Expression_ref (expr, 0)
      && ')' == tib_Expression_ref (expr, len-1))
    {
      count = 1;

      for (i = 1; i < len-1; ++i)
	{
	  if (is_left_paren (tib_Expression_ref (expr, i)))
	    ++count;

	  if (')' == tib_Expression_ref (expr, i))
	    --count;

	  if (0 == count)
	    break;
	}

      if (count > 0)
	return true;
    }

  return false;
}

bool
tib_eval_surrounded (const tib_Expression *expr)
{
  return tib_eval_surrounded_function (expr, '(');
}

bool
tib_eval_isnum (const tib_Expression *expr)
{
  size_t i, numdot = 0;

  for (i = 0; i < tib_Expression_len (expr); ++i)
    {
      int c = tib_Expression_ref (expr, i);

      if (('.' == c && ++numdot > 1) || !isdigit (c))
	return false;
    }

  return true;
}

bool
tib_eval_isstr (const tib_Expression *expr)
{
  size_t i, len = tib_Expression_len (expr);

  if (len > 1 && '"' == tib_Expression_ref (expr, 0))
    {
      for (i = 0; i < len-1; ++i)
	{
	  int c = tib_Expression_ref (expr, i);

	  if (c > 256 || '"' == c)
	    return false;
	}

      return true;
    }

  return false;
}

bool
tib_eval_islist (const tib_Expression *expr)
{
  size_t len = tib_Expression_len (expr);

  return (len > 2 && '{' == tib_Expression_ref (expr, 0)
	  && '}' == tib_Expression_ref (expr, len-1));
}

bool
tib_eval_ismatrix (const tib_Expression *expr)
{
  size_t i, len = tib_Expression_len (expr);

  if (len > 4 && '[' == tib_Expression_ref (expr, 0)
      && '[' == tib_Expression_ref (expr, 1)
      && ']' == tib_Expression_ref (expr, len-1))
    {
      size_t open_brackets = 1;

      for (i = 1; i < len-1; ++i)
	{
	  int c = tib_Expression_ref (expr, i);

	  if ('[' == c)
	    ++open_brackets;

	  if (']' == c)
	    --open_brackets;

	  if (0 == open_brackets || open_brackets > 2)
	    return false;
	}

      return true;
    }

  return false;
}

int
tib_eval_close_parens (tib_Expression *expr)
{
  size_t i, count = 0, len = tib_Expression_len (expr);

  for (i = 0; i < len; ++i)
    {
      int c = tib_Expression_ref (expr, i);

      if (is_left_paren (c))
	++count;

      if (')' == c)
	--count;
    }

  if (1 == count)
    return tib_Expression_push (expr, ')');

  if (count > 1)
    return TIB_EFORMAT;

  return 0;
}

int
tib_eval_parse_commas (const tib_Expression *expr, tib_Expression ***out,
		       size_t *out_len)
{
  int rc = 0;
  size_t i, j, last = 0, elements = 1, len = tib_Expression_len (expr);

  if (0 == len)
    {
      *out = NULL;
      *out_len = 0;
      return 0;
    }

  for (i = 0; i < len; ++i)
    if (',' == tib_Expression_ref (expr, i))
      ++elements;
  *out_len = elements;

  *out = malloc (elements * sizeof (tib_Expression *));
  if (NULL == *out)
    {
      *out_len = 0;
      return TIB_EALLOC;
    }

  for (i = 0; i <= elements; ++i)
    {
      for (j = last; j < len; ++j)
	if (',' == tib_Expression_ref (expr, j) || j == len-1)
	  {
	    rc = tib_Expression_substring (expr, &(*out[i]), last, j-1);
	    last = j + 1;
	    break;
	  }

      if (rc)
	break;
    }

  if (rc)
    {
      for (j = 0; j < i; ++j)
	tib_Expression_decref (*out[j]);
      free (*out);

      *out_len = 0;
      *out = NULL;
    }

  return rc;
}
