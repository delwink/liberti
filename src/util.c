/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016 Delwink, LLC
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

#include <stdarg.h>
#include <string.h>

#include "tibchar.h"
#include "util.h"

int
load_expr (struct tib_expr *dest, const char *src)
{
  unsigned int len = strlen (src);
  for (unsigned int i = 0; i < len; ++i)
    {
      int rc = tib_expr_push (dest, src[i]);
      if (rc)
        return rc;
    }

  return 0;
}

int
load_expr_num (struct tib_expr *dest, const char *src)
{
  int rc = load_expr (dest, src);
  if (rc)
    return rc;

  int e = tib_expr_indexof_r (dest, 'e');
  if (e >= 0)
    {
      tib_expr_delete (dest, e);
      dest->data[e] = TIB_CHAR_EPOW10;
    }

  return 0;
}

static const char *
display_special (int c)
{
  static const int SKIPS[] =
    {
      TIB_CHAR_DEGREE,
      TIB_CHAR_EPOW10,
      TIB_CHAR_GREATEREQUAL,
      TIB_CHAR_L1,
      TIB_CHAR_L2,
      TIB_CHAR_L3,
      TIB_CHAR_L4,
      TIB_CHAR_L5,
      TIB_CHAR_L6,
      TIB_CHAR_L7,
      TIB_CHAR_L8,
      TIB_CHAR_L9,
      TIB_CHAR_LESSEQUAL,
      TIB_CHAR_PI,
      TIB_CHAR_SMALL1,
      TIB_CHAR_SMALL2,
      TIB_CHAR_SMALL3,
      TIB_CHAR_SMALL4,
      TIB_CHAR_SMALL5,
      TIB_CHAR_SMALL6,
      TIB_CHAR_SMALL7,
      TIB_CHAR_SMALL8,
      TIB_CHAR_SMALL9,
      TIB_CHAR_SMALL_MINUS,
      TIB_CHAR_STO,
      TIB_CHAR_THETA
    };

  for (unsigned int i = 0; i < (sizeof SKIPS / sizeof (int)); ++i)
    if (SKIPS[i] == c)
      return NULL;

  return tib_special_char_text (c);
}

char *
get_expr_display_str (const struct tib_expr *expr)
{
  return tib_expr_tostr_f (expr, display_special);
}

int
max (int x, int y)
{
  return (x > y) ? x : y;
}

int
min (int x, int y)
{
  return (x < y) ? x : y;
}
