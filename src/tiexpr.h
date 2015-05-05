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

#ifndef DELWINK_TIBASIC_TISTRING_H
#define DELWINK_TIBASIC_TISTRING_H

#include <stdlib.h>

typedef struct {
  size_t len;
  int *value;
  size_t refs;
} tibasic_Expression;

tibasic_Expression *
tibasic_new_Expression (void);

void
tibasic_Expression_incref (tibasic_Expression *expr);

void
tibasic_Expression_decref (tibasic_Expression *expr);

int
tibasic_Expression_set (tibasic_Expression *expr, char *s);

void
tibasic_Expression_clear (tibasic_Expression *expr);

char *
tibasic_Expression_as_str (tibasic_Expression *expr);

int
tibasic_Expression_remove (tibasic_Expression *expr, size_t i);

int
tibasic_Expression_insert (tibasic_Expression *expr, size_t i, int c);

int
tibasic_Expression_substring (tibasic_Expression *in, tibasic_Expression **out,
			      size_t beg, size_t end);

int
tibasic_Expression_cat (tibasic_Expression *dest, tibasic_Expression *src);

int
tibasic_Expression_get_at (tibasic_Expression *expr, size_t i);

size_t
tibasic_Expression_len (tibasic_Expression *expr);

#endif
