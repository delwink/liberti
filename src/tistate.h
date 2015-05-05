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

#ifndef DELWINK_TIBASIC_TISTATE_H
#define DELWINK_TIBASIC_TISTATE_H

#include <stdlib.h>

#include "tistring.h"
#include "titype.h"

TI
tibasic_variable_get (tichar key);

void
tibasic_variable_set (tichar key, TI value);

bool
tibasic_variable_exists (tichar key);

TI
tibasic_Ans_get (void);

void
tibasic_Ans_set (TI value);

#endif
