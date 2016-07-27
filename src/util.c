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
max (int x, int y)
{
  return (x > y) ? x : y;
}
