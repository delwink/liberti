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

#include <stdlib.h>

#include "tiberr.h"
#include "tibvar.h"

struct varlist
{
  size_t len;
  tib_Variable *vars;
};

static struct varlist varlist =
  {
    .len = 0,
    .vars = NULL
  };

void
tib_var_free ()
{
  free (varlist.vars);

  varlist.len = 0;
  varlist.vars = NULL;
}

static int
add_var (int key, const TIB *value)
{
  tib_Variable *old = varlist.vars;

  ++varlist.len;
  varlist.vars = realloc (varlist.vars, varlist.len * sizeof (tib_Variable));
  if (NULL == varlist.vars)
    {
      varlist.vars = old;
      --varlist.len;
      return TIB_EALLOC;
    }

  tib_errno = 0;
  tib_Variable *new = varlist.vars + varlist.len - 1;
  new->key = key;
  new->value = tib_copy (value);
  if (tib_errno)
    --varlist.len;

  return tib_errno;
}

int
tib_var_set (int key, const TIB *value)
{
  if (!tib_is_var (key))
    return add_var (key, value);

  size_t i;
  for (i = 0; i < varlist.len; ++i)
    {
      if (key == varlist.vars[i].key)
	{
	  TIB *old = varlist.vars[i].value;
	  varlist.vars[i].value = tib_copy (value);
	  if (tib_errno)
	    {
	      varlist.vars[i].value = old;
	      return tib_errno;
	    }

	  tib_decref (old);
	  return 0;
	}
    }

  return TIB_EINDEX; /* should be unreachable */
}

TIB *
tib_var_get (int key)
{
  static const TIB zero =
    {
      .type = TIB_TYPE_COMPLEX,
      .value.number = { .dat = {0, 0} }
    };

  size_t i;
  for (i = 0; i < varlist.len; ++i)
    if (key == varlist.vars[i].key)
      return tib_copy (varlist.vars[i].value);

  return tib_copy (&zero);
}

bool
tib_is_var (int key)
{
  size_t i;
  for (i = 0; i < varlist.len; ++i)
    if (key == varlist.vars[i].key)
      return true;

  return false;
}
