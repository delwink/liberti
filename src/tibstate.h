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

#ifndef DELWINK_TIB_TIBSTATE_H
#define DELWINK_TIB_TIBSTATE_H

#include <stdlib.h>

#include "tibtype.h"

TIB
tibasic_variable_get (int key);

void
tibasic_variable_set (int key, TIB value);

bool
tibasic_variable_exists (int key);

TIB
tibasic_Ans_get (void);

void
tibasic_Ans_set (TIB value);

#endif
