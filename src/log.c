/*
 *  LiberTI - TI-like calculator designed for LibreCalc
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

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "log.h"

bool debug_mode = false;

#define log(FMT,FILE,PRE)                       \
  {                                             \
    fputs (PRE, FILE);                          \
    va_list ap;                                 \
    va_start (ap, FMT);                         \
    int n = vfprintf (FILE, FMT, ap);           \
    va_end (ap);                                \
    fputc ('\n', FILE);                         \
    return n;                                   \
  }

int
info (const char *fmt, ...)
{
  log (fmt, stdout, "[info]: ");
}

int
warn (const char *fmt, ...)
{
  log (fmt, stdout, "[warn]: ");
}

int
error (const char *fmt, ...)
{
  log (fmt, stderr, "[error]: ");
}

int
critical (const char *fmt, ...)
{
  log (fmt, stderr, "[critical]: ");
}

int
debug (const char *fmt, ...)
{
  if (!debug_mode)
    return 0;

  log (fmt, stdout, "[debug]: ");
}
