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

static gsl_rng *rng = NULL;

static struct registry registry =
  {
    .len = 0,
    .nodes = NULL
  };

static TIB *
func_paren (const struct tib_expr *expr)
{
  return tib_eval (expr);
}

static TIB *
func_sin (const struct tib_expr *expr)
{
  TIB *t = tib_eval (expr);
  if (!t)
    return NULL;

  if (tib_type (t) != TIB_TYPE_COMPLEX)
    {
      tib_errno = TIB_ETYPE;
      tib_decref (t);
      return NULL;
    }

  gsl_complex num = gsl_complex_sin (tib_complex_value (t));
  tib_decref (t);

  return tib_new_complex (GSL_REAL (num), GSL_IMAG (num));
}

static int
split_number_args (const struct tib_expr *expr, ...)
{
  const int *beg, *end;
  int rc = 0, numpar = 0;
  va_list ap;

  va_start (ap, expr);
  for (beg = expr->data, end = beg; end < expr->data + expr->len; ++end)
    {
      if (tib_is_func (*end))
        {
          ++numpar;
        }
      else if (')' == *end)
        {
          if (--numpar < 0)
            {
              rc = TIB_ESYNTAX;
              break;
            }
        }
      else if (0 == numpar &&
               (',' == *end || end + 1 == expr->data + expr->len))
        {
          int start = beg - expr->data, stop = end - expr->data - 1;
          if (end + 1 == expr->data + expr->len)
            ++stop;

          struct tib_expr arg;
          tib_subexpr (&arg, expr, start, stop);

          TIB *t = tib_eval (&arg);
          if (!t)
            {
              rc = tib_errno;
              break;
            }

          if (tib_type (t) != TIB_TYPE_COMPLEX)
            {
              tib_decref (t);
              tib_errno = TIB_ETYPE;
              break;
            }

          gsl_complex *out = va_arg (ap, gsl_complex *);
          *out = tib_complex_value (t);
          tib_decref (t);

          beg = end + 1;
        }
    }
  va_end (ap);

  return rc;
}

static int
is_int (gsl_complex z)
{
  return GSL_IMAG (z) == 0 && fmod (GSL_REAL (z), 1.0) == 0;
}

static TIB *
func_randint (const struct tib_expr *expr)
{
  int len = expr->len, num_commas = 0;

  for (int i = 0; i < len; ++i)
    {
      if (',' == expr->data[i])
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
  tib_errno = split_number_args (expr, &min, &max, &count);
  if (tib_errno)
    return NULL;

  if (!(is_int (min) && is_int (max) && is_int (count))
      || GSL_REAL (count) < 0)
    return NULL;

  double diff = GSL_REAL (max) - GSL_REAL (min);

  len = (unsigned int) GSL_REAL (count);
  gsl_complex vals[len];

  for (int i = 0; i < len; ++i)
    {
      GSL_SET_COMPLEX (&vals[i], (double) gsl_rng_get (rng), 0);

      while (GSL_REAL (vals[i]) < GSL_REAL (min))
        GSL_SET_REAL (&vals[i], GSL_REAL (vals[i]) + diff);
      while (GSL_REAL (vals[i]) > GSL_REAL (max))
        GSL_SET_REAL (&vals[i], GSL_REAL (vals[i]) - diff);
    }

  return tib_new_list (vals, len);
}

int
tib_registry_init ()
{
  int rc;

  if (registry.nodes != NULL || rng != NULL)
    tib_registry_free ();

  rng = gsl_rng_alloc (gsl_rng_taus2);
  if (NULL == rng)
    return TIB_EALLOC;

  gsl_rng_set (rng, (unsigned long) time (NULL));

#define ADD(K,F) rc = tib_registry_add (K, F); if (rc) goto fail;

  ADD ('(', func_paren);
  ADD (TIB_CHAR_SIN, func_sin);
  ADD (TIB_CHAR_RANDINT, func_randint);

#undef ADD

 fail:
  if (rc)
    tib_registry_free ();

  return rc;
}

void
tib_registry_free ()
{
  if (registry.nodes)
    free (registry.nodes);
  if (rng)
    gsl_rng_free (rng);

  registry.len = 0;
  registry.nodes = NULL;
  rng = NULL;
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
tib_call (int key, const struct tib_expr *expr)
{
  size_t i;
  for (i = 0; i < registry.len; ++i)
    if (key == registry.nodes[i].key)
      return registry.nodes[i].f (expr);

  tib_errno = TIB_EBADFUNC;
  return NULL;
}
