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
#include "tiblst.h"

struct tib_lst *
tib_new_lst ()
{
  struct tib_lst *out = malloc (sizeof (struct tib_lst));
  out->beg = NULL;
  out->end = NULL;

  return out;
}

static struct tib_el *
el_ref (const struct tib_lst *lst, size_t index)
{
  struct tib_el *i;
  size_t looped = 0;

  for (i = lst->beg; i != NULL; i = i->next)
    {
      if (looped++ == index)
        return i;
    }

  return NULL;
}

void
tib_free_lst (struct tib_lst *lst)
{
  size_t i;

  tib_foreachlst (lst, i)
    {
      tib_lst_remove (lst, 0);
    }

  free (lst);
}

int
tib_lst_insert (struct tib_lst *lst, TIB *t, size_t index)
{
  size_t len = tib_lst_len (lst);

  if (index > len)
    return TIB_EINDEX;

  struct tib_el *new = malloc (sizeof (struct tib_el));
  if (NULL == new)
    return TIB_EALLOC;

  TIB *newval = tib_copy (t);
  if (NULL == newval)
    {
      free (new);
      return TIB_EALLOC;
    }

  new->val = newval;

  if (0 == index)
    lst->beg = new;

  if (index == len)
    lst->end = new;

  new->next = el_ref (lst, index+1);
  new->prev = 0 == index ? NULL : el_ref (lst, index-1);

  if (new->next)
    new->next->prev = new;

  if (new->prev)
    new->prev->next = new;

  return 0;
}

int
tib_lst_push (struct tib_lst *lst, TIB *t)
{
  return tib_lst_insert (lst, t, tib_lst_len (lst));
}

void
tib_lst_remove (struct tib_lst *lst, size_t index)
{
  struct tib_el *e = el_ref (lst, index);

  if (e->next)
    e->prev->next = e->next;
  if (e->prev)
    e->next->prev = e->prev;

  free (e);
}

size_t
tib_lst_len (const struct tib_lst *lst)
{
  size_t i = 0;
  struct tib_el *e = lst->beg;

  while (e != NULL)
    {
      ++i;
      e = e->next;
    }

  return i;
}

TIB *
tib_lst_ref (const struct tib_lst *lst, size_t index)
{
  struct tib_el *i;
  size_t looped = 0;

  for (i = lst->beg; i != NULL; i = i->next)
    {
      if (looped++ == index)
        return i->val;
    }

  return NULL;
}
