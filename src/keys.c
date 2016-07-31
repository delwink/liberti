/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016 Delwink, LLC
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

#include "keys.h"
#include "tibchar.h"

static const SDL_Keycode UNCHANGED[] =
  {
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,

    SDLK_PERIOD,
    SDLK_MINUS,
    SDLK_SLASH,

    SDLK_i
  };

int
normalize_keycode (SDL_Keycode code, SDL_Keymod mod)
{
  if (mod & KMOD_SHIFT)
    {
      switch (code)
        {
        case SDLK_EQUALS:
          return '+';

        case SDLK_6:
          return '^';

        case SDLK_8:
          return '*';

        case SDLK_9:
          return '(';

        case SDLK_0:
          return ')';
        }
    }
  else
    {
      switch (code)
        {
        case SDLK_s:
          return TIB_CHAR_SIN;
        }
    }

  for (unsigned int i = 0; i < (sizeof UNCHANGED / sizeof (int)); ++i)
    if (code == UNCHANGED[i])
      return code;

  if (code >= SDLK_a && code <= SDLK_z)
    return code - SDLK_a + 'A';

  return 0;
}
