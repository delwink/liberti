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

#include <ctype.h>
#include <stdint.h>

#include "tibchar.h"
#include "tiberr.h"
#include "tibeval.h"
#include "tibfunction.h"
#include "tiblst.h"
#include "tibvar.h"

static bool
is_var_char (int c)
{
  return isupper (c) || TIB_CHAR_THETA == c;
}

static bool
needs_mult_common (int c)
{
  return (isdigit (c) || is_var_char (c));
}

static bool
needs_mult_right (int c)
{
  return (needs_mult_common (c) || tib_is_func (c));
}

static bool
needs_mult_left (int c)
{
  return (needs_mult_common (c) || ')' == c);
}

bool
is_sign_operator (int c)
{
  return ('+' == c || '-' == c);
}

static bool
is_math_operator (int c)
{
  return is_sign_operator (c) || '*' == c || '/' == c || '^' == c || '!' == c;
}

unsigned int
sign_count (const struct tib_expr *expr)
{
  int i;
  unsigned int out = 0;

  tib_expr_foreach (expr, i)
    if (is_sign_operator (expr->data[i]))
      ++out;

  return out;
}

bool
contains_i (const struct tib_expr *expr)
{
  int i;

  tib_expr_foreach (expr, i)
    if ('i' == expr->data[i])
      return true;

  return false;
}

static TIB *
single_eval (const struct tib_expr *expr)
{
  int len = expr->len;

  if (0 == len)
    return tib_empty ();

  if (1 == len && is_var_char (expr->data[0]))
    return tib_var_get (expr->data[0]);

  int func = tib_eval_surrounded (expr);
  if (func)
    {
      struct tib_expr temp;
      tib_subexpr (&temp, expr, 1, len - 1);
      return tib_call (func, &temp);
    }

  if (tib_eval_isnum (expr))
    {
      gsl_complex z;
      tib_errno = tib_expr_parse_complex (expr, &z);
      return tib_errno ? NULL : tib_new_complex (GSL_REAL (z), GSL_IMAG (z));
    }

  if (tib_eval_isstr (expr))
    {
      char *s = tib_expr_tostr (expr);
      if (NULL == s)
        return NULL;

      TIB *temp = tib_new_str (s);
      free (s);

      return temp;
    }

  tib_errno = TIB_ESYNTAX;
  return NULL;
}

static void
do_arith (struct tib_lst *resolved, size_t i, int operator, char arith1,
          TIB *(*func1) (const TIB *t1, const TIB *t2), char arith2,
          TIB *(*func2) (const TIB *t1, const TIB *t2))
{
  TIB *temp;

  if (arith1 == operator)
    temp = func1 (tib_lst_ref (resolved, i), tib_lst_ref (resolved, i + 1));
  else if (arith2 == operator)
    temp = func2 (tib_lst_ref (resolved, i), tib_lst_ref (resolved, i + 1));
  else
    return;

  if (NULL == temp)
    return;

  tib_lst_remove (resolved, i);
  tib_lst_remove (resolved, i);

  tib_errno = tib_lst_insert (resolved, temp, i);
  tib_decref (temp);
}

TIB *
tib_eval (const struct tib_expr *in)
{
  int i;

  if (0 == in->len)
    return tib_empty ();

  /* check for store operator */
  i = tib_expr_indexof (in, TIB_CHAR_STO);
  if (i >= 0)
    {
      if (i != in->len - 2 || 0 == i)
        {
          tib_errno = TIB_ESYNTAX;
          return NULL;
        }

      int c = in->data[i + 1];
      if (!is_var_char (c))
        {
          tib_errno = TIB_ESYNTAX;
          return NULL;
        }

      struct tib_expr e;
      tib_subexpr (&e, in, 0, i);

      TIB *stoval = tib_eval (&e);
      if (NULL == stoval)
        return NULL;

      tib_errno = tib_var_set (c, stoval);
      if (tib_errno)
        {
          tib_decref (stoval);
          return NULL;
        }

      return stoval;
    }

  struct tib_expr expr = { .bufsize = 0 };
  tib_errno = tib_exprcpy (&expr, in);
  if (tib_errno)
    return NULL;

  /* check for implicit closing parentheses and close them */
  tib_errno = tib_eval_close_parens (&expr);
  if (tib_errno)
    {
      tib_expr_destroy (&expr);
      return NULL;
    }

  /* add multiplication operators between implicit multiplications */
  bool add = true;
  for (i = 0; i < expr.len; ++i)
    {
      int c = expr.data[i];

      if ('"' == c)
        {
          add = !add; /* don't change anything inside a string */
        }
      else if (add)
        {
          if (is_var_char (c))
            {
              if (i > 0 && needs_mult_left (expr.data[i - 1]))
                tib_errno = tib_expr_insert (&expr, i++, '*');

              if (!tib_errno && i < expr.len - 1
                  && needs_mult_right (expr.data[i + 1]))
                tib_errno = tib_expr_insert (&expr, ++i, '*');
            }
          else if (i > 0 && i < expr.len - 1)
            {
              if (tib_is_func (c) && needs_mult_left (expr.data[i - 1]))
                tib_errno = tib_expr_insert (&expr, i++, '*');
              else if (')' == c && needs_mult_right (expr.data[i + 1]))
                tib_errno = tib_expr_insert (&expr, ++i, '*');
            }

          if (tib_errno)
            {
              tib_expr_destroy (&expr);
              return NULL;
            }
        }
    }

  /* this is temp storage for internally-resolved portions */
  struct tib_lst *resolved = tib_new_lst ();
  if (!resolved)
    {
      tib_expr_destroy (&expr);
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  /* this is to remember the operations to be executed */
  struct tib_expr calc;
  tib_errno = tib_expr_init (&calc);
  if (tib_errno)
    {
      tib_expr_destroy (&expr);
      tib_free_lst (resolved);
      return NULL;
    }

  /* resolve operand expressions, and store the values for later */
  int beg = 0, numpar = 0;
  tib_expr_foreach (&expr, i)
    {
      int c = expr.data[i];

      if (tib_is_func (c))
        {
          ++numpar;
        }
      else if (')' == c)
        {
          if (0 == numpar)
            {
              tib_errno = TIB_ESYNTAX;
              break;
            }

          --numpar;
        }

      if (0 == numpar && is_math_operator (c))
        {
          struct tib_expr sub;
          tib_subexpr (&sub, &expr, beg, i);

          TIB *temp = single_eval (&sub);
          if (!temp)
            break;

          if ('!' == c)
            {
              do
                {
                  TIB *fact = tib_factorial (temp);
                  tib_decref (temp);
                  if (!fact)
                    break;

                  temp = fact;
                }
              while (++i < expr.len && '!' == (c = expr.data[i]));

              if (tib_errno)
                break;

              tib_errno = tib_lst_push (resolved, temp);
              tib_decref (temp);
              if (tib_errno)
                break;

              if (i < expr.len)
                {
                  c = expr.data[i];

                  if (is_math_operator (c))
                    {
                      tib_errno = tib_expr_push (&calc, c);
                      if (tib_errno)
                        break;

                      ++i;
                    }
                  else
                    {
                      tib_errno = tib_expr_push (&calc, '*');
                      if (tib_errno)
                        break;
                    }

                  beg = i;
                }
            }
          else
            {
              tib_errno = tib_lst_push (resolved, temp);
              tib_decref (temp);
              if (tib_errno)
                break;

              tib_errno = tib_expr_push (&calc, c);
              if (tib_errno)
                break;

              beg = i + 1;
            }
        }
    }

  if (!tib_errno)
    {
      if (tib_lst_len (resolved) == 0)
        {
          TIB *temp = single_eval (&expr);
          if (!temp)
            goto end;

          tib_errno = tib_lst_push (resolved, temp);
          tib_decref (temp);
          if (tib_errno)
            goto end;
        }
      else if (expr.data[expr.len - 1] != '!')
        {
          struct tib_expr sub;
          tib_subexpr (&sub, &expr, beg, i);

          TIB *temp = single_eval (&sub);
          if (!temp)
            goto end;

          tib_errno = tib_lst_push (resolved, temp);
          tib_decref (temp);
          if (tib_errno)
            goto end;
        }

      if (tib_lst_len (resolved) != calc.len + 1)
        tib_errno = TIB_ESYNTAX;
    }

  tib_expr_destroy (&expr);

  if (tib_errno)
    goto end;

  tib_expr_foreach (&calc, i)
    {
      if ('^' == calc.data[i])
        {
          TIB *power = tib_lst_ref (resolved, i + 1);
          if (TIB_TYPE_COMPLEX != tib_type (power))
            {
              tib_errno = TIB_ETYPE;
              goto end;
            }

          TIB *temp = tib_pow (tib_lst_ref (resolved, i),
                               tib_complex_value (power));
          if (!temp)
            goto end;

          tib_lst_remove (resolved, i);
          tib_lst_remove (resolved, i);

          tib_errno = tib_lst_insert (resolved, temp, i);
          tib_decref (temp);
          if (tib_errno)
            goto end;

          tib_expr_delete (&calc, i--);
        }
    }

  int orig_len = tib_lst_len (resolved);
  tib_expr_foreach (&calc, i)
    {
      do_arith (resolved, i - (orig_len - tib_lst_len (resolved)),
                calc.data[i], '*', tib_mul, '/', tib_div);
      if (tib_errno)
        goto end;
    }

  tib_expr_foreach (&calc, i)
    {
      do_arith (resolved, 0, calc.data[i], '+', tib_add, '-', tib_sub);
      if (tib_errno)
        goto end;
    }

 end:
  tib_expr_destroy (&calc);

  TIB *out = NULL;
  if (!tib_errno)
    {
      out = tib_lst_ref (resolved, 0);
      tib_incref (out);
    }

  tib_free_lst (resolved);

  return out;
}

int
tib_eval_surrounded (const struct tib_expr *expr)
{
  int count = 0, opening = expr->data[0], len = expr->len;

  if (len > 2 && tib_is_func (opening) && ')' == expr->data[len - 1])
    {
      count = 1;

      for (int i = 1; i < len-1; ++i)
        {
          int c = expr->data[i];

          if (tib_is_func (c))
            ++count;
          else if (')' == c && --count == 0)
            return 0;
        }

      return opening;
    }

  return 0;
}

static int
char_count (const struct tib_expr *expr, int c)
{
  int i, count = 0;

  tib_expr_foreach (expr, i)
    if (c == expr->data[i])
      ++count;

  return count;
}

int
i_count (const struct tib_expr *expr)
{
  return char_count (expr, 'i');
}

static int
dot_count (const struct tib_expr *expr)
{
  return char_count (expr, '.');
}

static bool
is_number_char (int c)
{
  return (isdigit (c) || '.' == c || 'i' == c || is_sign_operator (c));
}

int
get_char_pos (const struct tib_expr *expr, int c, int which)
{
  int i, found = 0;

  tib_expr_foreach (expr, i)
    {
      if (c == expr->data[i] && ++found == which)
        break;
    }

  return i;
}

static int
get_sign_pos (const struct tib_expr *expr, int which)
{
  int i, found = 0;

  tib_expr_foreach (expr, i)
    {
      if (is_sign_operator (expr->data[i]) && ++found == which)
        break;
    }

  return i;
}

static bool
good_sign_pos (const struct tib_expr *expr, int numsign, int numi)
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
tib_eval_isnum (const struct tib_expr *expr)
{
  int signs = sign_count (expr);
  int dots = dot_count (expr);
  int is = i_count (expr);

  if (signs > 2 || dots > 2 || is > 1)
    return false;

  if (!good_sign_pos (expr, signs, is))
    return false;

  if (is && get_char_pos (expr, 'i', 1) < expr->len - 1)
    return false;

  int i;
  tib_expr_foreach (expr, i)
    if (!is_number_char (expr->data[i]))
      return false;

  return true;
}

bool
tib_eval_isstr (const struct tib_expr *expr)
{
  int len = expr->len;

  if (len > 1 && '"' == expr->data[0])
    {
      for (int i = 1; i < len-1; ++i)
        {
          int c = expr->data[i];

          if (c > 127 || '"' == c)
            return false;
        }

      return true;
    }

  return false;
}

bool
tib_eval_islist (const struct tib_expr *expr)
{
  int len = expr->len;

  if (len > 2 && '{' == expr->data[0] && '}' == expr->data[len - 1])
    {
      for (int i = 1; i < len-1; ++i)
        {
          int c = expr->data[i];

          if ('{' == c || '}' == c)
            return false;
        }

      return true;
    }

  return false;
}

static bool
sub_isnum (const struct tib_expr *expr, int beg, int end)
{
  if (end <= beg)
    return false;

  struct tib_expr temp;
  int rc = tib_subexpr (&temp, expr, beg, end);
  if (rc)
    return false;

  return tib_eval_isnum (&temp);
}

bool
tib_eval_ismatrix (const struct tib_expr *expr)
{
  int len = expr->len;

  if (len > 4 && '[' == expr->data[0] && '[' == expr->data[1]
      && ']' == expr->data[len - 1])
    {
      int i = 2;
      int open_brackets = i, fdim = 1, dim = 1, beg = i, end = i;
      bool first = true;

      for (; i < len; ++i)
        {
          int c = expr->data[i];

          switch (c)
            {
            case '[':
              ++open_brackets;
              beg = i + 1;
              break;

            case ']':
              --open_brackets;

              if (!first && dim != fdim)
                return false;

              first = false;
              dim = 1;
              end = i - 1;

              if (!sub_isnum (expr, beg, end))
                return false;
              break;

            case ',':
              if (first)
                ++fdim;
              else
                ++dim;

              end = i - 1;
              if (!sub_isnum (expr, beg, end))
                return false;
              beg = i + 1;
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
tib_eval_close_parens (struct tib_expr *expr)
{
  int count = 0, len = expr->len;
  bool str = false;

  for (int i = 0; i < len; ++i)
    {
      int c = expr->data[i];

      if ('"' == c)
        str = !str;

      if (!str)
        {
          if (tib_is_func (c))
            ++count;

          if (')' == c)
            --count;
        }
    }

  while (count--)
    {
      int rc = tib_expr_push (expr, ')');
      if (rc)
        return rc;
    }

  return 0;
}
