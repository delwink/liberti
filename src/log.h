/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2015, 2017 Delwink, LLC
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

#ifndef DELWINK_LIBERTI_LOG_H
#define DELWINK_LIBERTI_LOG_H

#include <stdbool.h>

extern bool debug_mode;

int
info(const char *fmt, ...);

int
warn(const char *fmt, ...);

int
error(const char *fmt, ...);

int
critical(const char *fmt, ...);

int
debug(const char *fmt, ...);

#endif
