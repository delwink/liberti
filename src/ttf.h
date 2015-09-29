/*
 *  LiberTI - Libre TI calculator emulator designed for LibreCalc
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

#ifndef DELWINK_LIBERTI_TTF_H
#define DELWINK_LIBERTI_TTF_H

#include <SDL_ttf.h>

struct fontset
{
  TTF_Font *reg;
  TTF_Font *small;
};

struct fontset *
get_font_set (int scale);

void
free_font_set (struct fontset *fs);

#endif
