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

#ifndef DELWINK_TIB_H
#define DELWINK_TIB_H

enum tib_err
  {
    TIB_EALLOC   = -1,
    TIB_EINDEX   = -2,
    TIB_ESYNTAX  = -3,
    TIB_ETYPE    = -4,
    TIB_EDIM     = -5,
    TIB_ENULLPTR = -6,
    TIB_EDOMAIN  = -7
  };

extern int tib_errno;

#endif
