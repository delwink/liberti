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
tib_eval_surrounded (tib_Expression *expr)
{
  int count = 0;
  size_t i, len = tib_Expression_len (expr);

  if (len > 2 && '(' == tib_Expression_get_at (expr, 0)
	&& ')' == tib_Expression_get_at (expr, len-1))
    {
      count = 1;

      for (i = 0; i < len; ++i)
	{
	  if (is_left_paren (tib_Expression_get_at (expr, i)))
	    ++count;

	  if (')' == tib_Expression_get_at (expr, i))
	    --count;

	  if (0 == count)
	    break;
	}

      if (count > 0)
	return true;
    }

  return false;
}
