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

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_rng.h>

#include "tibchar.h"
#include "tiberr.h"
#include "tibeval.h"
#include "tibfunction.h"

struct registry_node
{
  int key;
  tib_Function f;
};

struct registry
{
  size_t len;
  struct registry_node *nodes;
};

static struct registry registry =
  {
    .len = 0,
    .nodes = NULL
  };

static TIB *
func_paren (const tib_Expression *expr)
{
  return tib_eval (expr);
}

static TIB *
func_sin (const tib_Expression *expr)
{
  gsl_complex num;

  tib_errno = tib_Expression_as_num (expr, &num);
  if (tib_errno)
    return NULL;

  num = gsl_complex_sin (num);

  return tib_new_complex (GSL_REAL (num), GSL_IMAG (num));
}

static int
split_number_args (const tib_Expression *expr, ...)
{
  const int *beg, *end;
  va_list ap;

  tib_errno = 0;

  va_start (ap, expr);
  for (beg = expr->value, end = beg; end < expr->value + expr->len; ++end)
    {
      if (',' == *end || end + 1 == expr->value + expr->len)
	{
	  size_t start = beg - expr->value, stop = end - expr->value - 1;
	  if (end + 1 == expr->value + expr->len)
	    ++stop;

	  tib_Expression *arg = tib_Expression_substring (expr, start, stop);
	  if (NULL == arg)
	    break;

	  gsl_complex *out = va_arg (ap, gsl_complex *);
	  tib_errno = tib_Expression_as_num (arg, out);
	  tib_Expression_decref (arg);
	  if (tib_errno)
	    break;

	  beg = end + 1;
	}
    }
  va_end (ap);

  return tib_errno;
}

static int
is_int (gsl_complex z)
{
  return GSL_IMAG (z) == 0 && fmod (GSL_REAL (z), 1.0) == 0;
}

static TIB *
func_randint (const tib_Expression *expr)
{
  size_t i, len = tib_Expression_len (expr), num_commas = 0;
  for (i = 0; i < len; ++i)
    {
      if (',' == tib_Expression_ref (expr, i))
	{
	  if (++num_commas > 2)
	    break;
	}
    }

  if (num_commas != 2)
    {
      tib_errno = TIB_EARGNUM;
      return NULL;
    }

  gsl_complex min, max, count;
  split_number_args (expr, &min, &max, &count);
  if (tib_errno)
    return NULL;

  if (!(is_int (min) && is_int (max) && is_int (count))
      || GSL_REAL (count) < 0)
    return NULL;

  double diff = GSL_REAL (max) - GSL_REAL (min);

  gsl_rng *rng = gsl_rng_alloc (gsl_rng_taus2);
  if (NULL == rng)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  gsl_rng_set (rng, (unsigned long) time (NULL));

  len = (size_t) GSL_REAL (count);
  gsl_complex vals[len];
  for (i = 0; i < len; ++i)
    {
      GSL_SET_COMPLEX (&vals[i], (double) gsl_rng_get (rng), 0);

      while (GSL_REAL (vals[i]) < GSL_REAL (min))
	GSL_SET_REAL (&vals[i], GSL_REAL (vals[i]) + diff);
      while (GSL_REAL (vals[i]) > GSL_REAL (max))
	GSL_SET_REAL (&vals[i], GSL_REAL (vals[i]) - diff);
    }

  gsl_rng_free (rng);

  return tib_new_list (vals, len);
}

int
tib_registry_init ()
{
  int rc;

  if (registry.nodes != NULL)
    tib_registry_free ();

#define ADD(K,F) rc = tib_registry_add (K, F); if (rc) goto fail;

  ADD('(', func_paren);
  ADD(TIB_CHAR_SIN, func_sin);
  ADD(TIB_CHAR_RANDINT, func_randint);

#undef ADD

 fail:
  if (rc)
    tib_registry_free ();
  return rc;
}

void
tib_registry_free ()
{
  free (registry.nodes);

  registry.len = 0;
  registry.nodes = NULL;
}

int
tib_registry_add (int key, tib_Function f)
{
  struct registry_node *old = registry.nodes;

  ++registry.len;
  registry.nodes = realloc (registry.nodes,
			    registry.len * sizeof (struct registry_node));
  if (NULL == registry.nodes)
    {
      registry.nodes = old;
      --registry.len;
      return TIB_EALLOC;
    }

  struct registry_node new =
    {
      .key = key,
      .f = f
    };

  registry.nodes[registry.len - 1] = new;
  return 0;
}

bool
tib_is_func (int key)
{
  size_t i;
  for (i = 0; i < registry.len; ++i)
    if (key == registry.nodes[i].key)
      return true;

  return false;
}

TIB *
tib_call (int key, const tib_Expression *expr)
{
  size_t i;
  for (i = 0; i < registry.len; ++i)
    if (key == registry.nodes[i].key)
      return registry.nodes[i].f (expr);

  tib_errno = TIB_EBADFUNC;
  return NULL;
}
