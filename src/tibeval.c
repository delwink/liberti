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

enum math_operator_function_type
  {
    T,
    TC,
    TT
  };

struct math_operator
{
  union
  {
    TIB *(*t) (const TIB *);
    TIB *(*tc) (const TIB *, gsl_complex);
    TIB *(*tt) (const TIB *, const TIB *);
  } func;

  enum math_operator_function_type function_type;

  int c;
  int priority;
};

static const struct math_operator OPERATORS[] =
  {
    { { .t = tib_factorial }, T,  '!', 0 },
    { { .tc = tib_pow },      TC, '^', 1 },
    { { .tt = tib_mul },      TT, '*', 2 },
    { { .tt = tib_div },      TT, '/', 2 },
    { { .tt = tib_add },      TT, '+', 3 },
    { { .tt = tib_sub },      TT, '-', 3 }
  };

#define NUM_MATH_OPERATORS (sizeof OPERATORS / sizeof (struct math_operator))
#define LAST_PRIORITY 3

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

static const struct math_operator *
get_math_operator (int c)
{
  for (unsigned int i = 0; i < NUM_MATH_OPERATORS; ++i)
    if (OPERATORS[i].c == c)
      return &OPERATORS[i];

  return NULL;
}

static bool
is_math_operator (int c)
{
  return get_math_operator (c) != NULL;
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

static int
replace_epow10 (struct tib_expr *expr)
{
  int i;

  tib_expr_foreach (expr, i)
    {
      if (TIB_CHAR_EPOW10 == expr->data[i])
        {
          expr->data[i++] = '*';

          const char *s = "10^";
          for (; *s != '\0'; ++s, ++i)
            {
              int rc = tib_expr_insert (expr, i, *s);
              if (rc)
                return rc;
            }
        }
    }

  return 0;
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

  /* replace power of 10 E character with "*10^" */
  tib_errno = replace_epow10 (&expr);
  if (tib_errno)
    {
      tib_expr_destroy (&expr);
      return NULL;
    }

  /* add multiplication operators between implicit multiplications */
  bool add = true;
  tib_expr_foreach (&expr, i)
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
  add = true;
  tib_expr_foreach (&expr, i)
    {
      int c = expr.data[i];

      if ('"' == c)
        {
          add = !add;
          continue;
        }

      if (!add)
        continue;

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

      const struct math_operator *oper;
      if (0 == numpar && (oper = get_math_operator (c)) != NULL)
        {
          struct tib_expr sub;
          tib_subexpr (&sub, &expr, beg, i);

          TIB *part = single_eval (&sub);
          if (!part)
            break;

          if (T == oper->function_type)
            {
              do
                {
                  TIB *temp = oper->func.t (part);
                  tib_decref (part);
                  if (!temp)
                    break;

                  part = temp;
                }
              while (++i < expr.len
                     && (oper = get_math_operator (expr.data[i])) != NULL
                     && T == oper->function_type);

              if (tib_errno)
                break;

              tib_errno = tib_lst_push (resolved, part);
              tib_decref (part);
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
              tib_errno = tib_lst_push (resolved, part);
              tib_decref (part);
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
      const struct math_operator *oper;

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
      else if (NULL == (oper = get_math_operator (expr.data[expr.len - 1]))
               || oper->function_type != T)
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

  for (int priority = 0; priority <= LAST_PRIORITY; ++priority)
    {
      for (i = 0; i < calc.len; ++i)
        {
          const struct math_operator *oper = get_math_operator (calc.data[i]);
          if (oper->priority != priority)
            continue;

          TIB *t;
          unsigned int num_operands;
          switch (oper->function_type)
            {
            case TC:
              t = tib_lst_ref (resolved, i + 1);
              if (tib_type (t) != TIB_TYPE_COMPLEX)
                {
                  tib_errno = TIB_ETYPE;
                  goto end;
                }

              t = oper->func.tc (tib_lst_ref (resolved, i),
                                 tib_complex_value (t));
              if (!t)
                goto end;

              num_operands = 2;
              break;

            case TT:
              t = oper->func.tt (tib_lst_ref (resolved, i),
                                 tib_lst_ref (resolved, i + 1));
              if (!t)
                goto end;

              num_operands = 2;
              break;

            default:
              tib_errno = TIB_ESYNTAX;
              goto end;
            }

          for (unsigned int _ = 0; _ < num_operands; ++_)
            tib_lst_remove (resolved, i);

          tib_errno = tib_lst_insert (resolved, t, i);
          tib_decref (t);

          if (tib_errno)
            goto end;

          tib_expr_delete (&calc, i--);
        }
    }

 end:
  tib_expr_destroy (&calc);

  TIB *out = NULL;
  if (!tib_errno)
    {
      if (tib_lst_len (resolved) != 1)
        {
          tib_errno = TIB_ESYNTAX;
        }
      else
        {
          out = tib_lst_ref (resolved, 0);
          tib_incref (out);
        }
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
