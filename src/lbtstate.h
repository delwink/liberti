/*
 *  libliberti - Backend functionality for LiberTI
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

#ifndef DELWINK_LIBLIBERTI_STATE_H
#define DELWINK_LIBLIBERTI_STATE_H

#include <libconfig.h>
#include <stdlib.h>

typedef struct
{
  size_t refs;
  char *save_path;
  config_t conf;
} lbt_State;

lbt_State *
lbt_new_State (const char *save_path);

void
lbt_State_incref (lbt_State *self);

void
lbt_State_decref (lbt_State *self);

#endif