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

#include <ctype.h>
#include <string.h>

#include "tiberr.h"
#include "tibeval.h"
#include "tibchar.h"
#include "tiblst.h"

#define TIB_FORMAT_CHAR 999

static bool
is_left_paren (int c)
{
  return ('(' == c || TIB_CHAR_SIN == c || TIB_CHAR_RANDINT == c
	  || TIB_CHAR_NOT == c || TIB_CHAR_INT == c || TIB_CHAR_DIM == c
	  || TIB_CHAR_PIXEL_TEST == c);
}

static bool
needs_mult_common (int c)
{
  return (isdigit (c));
}

static bool
needs_mult_right (int c)
{
  return (needs_mult_common (c) || '(' == c);
}

static bool
needs_mult_left (int c)
{
  return (needs_mult_common (c) || ')' == c);
}

static bool
sign_operator (int c)
{
  return ('+' == c || '-' == c);
}

static size_t
sign_count (const tib_Expression *expr)
{
  size_t i, out = 0;

  tib_foreachexpr (expr, i)
    if (sign_operator (tib_Expression_ref (expr, i)))
      ++out;

  return out;
}

static bool
contains_i (const tib_Expression *expr)
{
  size_t i;

  tib_foreachexpr (expr, i)
    if ('i' == tib_Expression_ref (expr, i))
      return true;

  return false;
}

int
eval (tib_Expression *expr, TIB **out)
{
  int rc;
  size_t i, len = tib_Expression_len (expr);

  if (0 == len)
    {
      *out = tib_empty ();
      return 0;
    }

  /* check for implicit closing parentheses and close them */
  rc = tib_eval_close_parens (expr);
  if (rc)
    {
      *out = NULL;
      return rc;
    }

  /* if the expression is a valid number, resolve it and return */
  if (tib_eval_isnum (expr))
    {
      rc = 0;
      size_t numop = sign_count (expr);

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

      double real, imag;

      if (s == i_start)
	real = 0;
      else
	real = strtod (s, NULL);

      if (s == i_start)
	{
	  if (sign_operator (*i_start) && 'i' == i_start[1])
	    i_start[1] = '1';
	  else if ('i' == *i_start)
	    *i_start = '1';

	  imag = strtod (i_start, NULL);
	}
      else
	{
	  imag = strtod (i_start, NULL);
	}

      free (s);

      *out = tib_new_complex (real, imag);
      if (NULL == *out)
	rc = TIB_EALLOC;

      return rc;
    }

  /* if the expression is a valid string, resolve it and return */
  if (tib_eval_isstr (expr))
    {
      char *s = tib_Expression_as_str (expr);
      if (NULL == s)
	return TIB_EALLOC;

      *out = tib_new_str (s);
      free (s);

      if (NULL == *out)
	return TIB_EALLOC;

      return 0;
    }

  /* add multiplication operators between implicit multiplications */
  bool add = true;
  for (i = 1; i < len-1; ++i)
    {
      int c = tib_Expression_ref (expr, i);

      if ('"' == c)
	add = !add; /* don't change anything inside a string */

      if (add)
	{
	  if (is_left_paren (c) && i
	      && needs_mult_left (tib_Expression_ref (expr, i)))
	    rc = tib_Expression_insert (expr, i, '*');
	  else if (')' == c
		   && needs_mult_right (tib_Expression_ref (expr, i+1)))
	    rc = tib_Expression_insert (expr, i+1, '*');
	  else
	    rc = 0;

	  if (rc)
	    return rc;
	}
    }

  /* this is temp storage for internally-resolved portions */
  struct tib_lst *resolved = tib_new_lst ();
  if (NULL == resolved)
    return TIB_EALLOC;

  /* resolve divided expressions, and store the values for later */
  /* TODO: test for more than just parenthesized expressions */
  for (i = 0; i < len; ++i)
    {
      int c = tib_Expression_ref (expr, i);

      if ('(' == c)
	{
	  expr->value[i] = TIB_FORMAT_CHAR;

	  size_t close, numpar = 1;
	  for (close = i+1; close < len; ++close)
	    {
	      int temp = tib_Expression_ref (expr, close);

	      if ('(' == temp)
		++numpar;

	      if (')' == temp && --numpar == 0)
		break;
	    }

	  if (numpar)
	    {
	      rc = TIB_ESYNTAX;
	      break;
	    }

	  tib_Expression *sub;
	  rc = tib_Expression_substring (expr, &sub, i+1, close);
	  if (rc)
	    break;

	  TIB *res;
	  rc = eval (sub, &res);
	  tib_Expression_decref (sub);
	  if (rc)
	    break;

	  rc = tib_lst_push (resolved, res);
	  tib_decref (res);
	}
    }

  if (rc)
    {
      tib_free_lst (resolved);
      return rc;
    }

  /* TODO: loop through resolved parts and do arithmetic */

  return TIB_ESYNTAX;
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

static size_t
char_count (const tib_Expression *expr, int c)
{
  size_t i, count = 0;

  tib_foreachexpr (expr, i)
    if (c == tib_Expression_ref (expr, i))
      ++count;

  return count;
}

static size_t
i_count (const tib_Expression *expr)
{
  return char_count (expr, 'i');
}

static size_t
dot_count (const tib_Expression *expr)
{
  return char_count (expr, '.');
}

static bool
is_number_char (int c)
{
  return (isdigit (c) || '.' == c || 'i' == c || sign_operator (c));
}

static size_t
get_char_pos (const tib_Expression *expr, int c, size_t which)
{
  size_t i, found = 0;

  tib_foreachexpr (expr, i)
    {
      if (c == tib_Expression_ref (expr, i))
	++found;

      if (found == which)
	break;
    }

  return i;
}

static size_t
get_sign_pos (const tib_Expression *expr, size_t which)
{
  size_t i, found = 0;

  tib_foreachexpr (expr, i)
    {
      if (sign_operator (tib_Expression_ref (expr, i)))
	++found;

      if (found == which)
	break;
    }

  return i;
}

static bool
good_sign_pos (const tib_Expression *expr, size_t numsign, size_t numi)
{
  switch (numsign)
    {
    case 0:
      return true;

    case 1:
      if (!numi && get_sign_pos (expr, 1) != 0)
	return false;
      break;

    case 2:
      if (!numi || get_sign_pos (expr, 1) != 0
	  || get_sign_pos (expr, 2) > get_char_pos (expr, 'i', 1))
	return false;
      break;

    default:
      return false;
    }

  return true;
}

bool
tib_eval_isnum (const tib_Expression *expr)
{
  size_t i;
  size_t nums[3] = {
    sign_count (expr),
    dot_count (expr),
    i_count (expr)
  };

  for (i = 0; i < 2; ++i)
    if (nums[i] > 2)
      return false;

  if (nums[2] > 1)
    return false;

  if (!good_sign_pos (expr, nums[0], nums[2]))
    return false;

  if (nums[2] && get_char_pos (expr, 'i', 1) < tib_Expression_len (expr) - 1)
    return false;

  tib_foreachexpr (expr, i)
    if (!is_number_char (tib_Expression_ref (expr, i)))
      return false;

  return true;
}

bool
tib_eval_isstr (const tib_Expression *expr)
{
  size_t i, len = tib_Expression_len (expr);

  if (len > 1 && '"' == tib_Expression_ref (expr, 0))
    {
      for (i = 1; i < len-1; ++i)
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
  size_t i, len = tib_Expression_len (expr);

  if (len > 2 && '{' == tib_Expression_ref (expr, 0)
      && '}' == tib_Expression_ref (expr, len-1))
    {
      for (i = 1; i < len-1; ++i)
	{
	  int c = tib_Expression_ref (expr, i);

	  if ('{' == c || '}' == c)
	    return false;
	}

      return true;
    }

  return false;
}

static bool
sub_isnum (const tib_Expression *expr, size_t beg, size_t end)
{
  int rc;
  tib_Expression *temp;
  bool out;

  if (end > beg)
    return false;

  rc = tib_Expression_substring (expr, &temp, beg, end);
  if (rc)
    return false;

  out = tib_eval_isnum (temp);
  tib_Expression_decref (temp);
  return out;
}

bool
tib_eval_ismatrix (const tib_Expression *expr)
{
  size_t i, len = tib_Expression_len (expr);

  if (len > 4 && '[' == tib_Expression_ref (expr, 0)
      && '[' == tib_Expression_ref (expr, 1)
      && ']' == tib_Expression_ref (expr, len-1))
    {
      size_t open_brackets = 1, fdim = 1, dim = 1, beg = 1, end = 1;
      bool first = true;

      for (i = 1; i < len; ++i)
	{
	  int c = tib_Expression_ref (expr, i);

	  switch (c)
	    {
	    case '[':
	      ++open_brackets;
	      beg = i+1;
	      break;

	    case ']':
	      --open_brackets;

	      if (!first && dim != fdim)
		return false;

	      first = false;
	      dim = 1;
	      end = i-1;

	      if (!sub_isnum (expr, beg, end))
		return false;
	      break;

	    case ',':
	      if (first)
		++fdim;
	      else
		++dim;

	      end = i-1;
	      if (!sub_isnum (expr, beg, end))
		return false;
	      beg = i+1;
	      break;
	    }

	  if ((0 == open_brackets && i != len-1) || open_brackets > 2)
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
  bool str = false;

  for (i = 0; i < len; ++i)
    {
      int c = tib_Expression_ref (expr, i);

      if ('"' == c)
	str = !str;

      if (!str)
	{
	  if (is_left_paren (c))
	    ++count;

	  if (')' == c)
	    --count;
	}
    }

  if (1 == count)
    return tib_Expression_push (expr, ')');

  if (count > 1)
    return TIB_ESYNTAX;

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
