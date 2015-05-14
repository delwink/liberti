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

#ifndef DELWINK_TIB_LST_H
#define DELWINK_TIB_LST_H

#include "tibtype.h"

#define tib_foreachlst(L,I) for (I = 0; i < tib_lst_len (L); ++I)

struct tib_el
{
  TIB *val;
  struct tib_el *next;
  struct tib_el *prev;
};

struct tib_lst
{
  struct tib_el *beg;
  struct tib_el *end;
};

struct tib_lst *
tib_new_lst (void);

void
tib_free_lst (struct tib_lst *lst);

int
tib_lst_insert (struct tib_lst *lst, TIB *t, size_t index);

int
tib_lst_push (struct tib_lst *lst, TIB *t);

void
tib_lst_remove (struct tib_lst *lst, size_t index);

size_t
tib_lst_len (const struct tib_lst *lst);

TIB *
tib_lst_ref (const struct tib_lst *lst, size_t index);

#endif
