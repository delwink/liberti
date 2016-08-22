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

static const SDL_Keycode UNCHANGED_RANGES[] =
  {
    SDLK_0, SDLK_9
  };

static const SDL_Keycode UNCHANGED[] =
  {
    SDLK_PERIOD,
    SDLK_MINUS,
    SDLK_SLASH,

    SDLK_e,
    SDLK_i
  };

int
normalize_keycode (SDL_Keycode code, SDL_Keymod mod)
{
  if (mod & KMOD_CTRL)
    {
      switch (code)
        {
        case SDLK_e:
          return TIB_CHAR_EPOW10;
        }
    }
  else if (mod & KMOD_SHIFT)
    {
      switch (code)
        {
        case SDLK_EQUALS:
          return '+';

        case SDLK_1:
          return '!';

        case SDLK_4:
          return TIB_CHAR_STO;

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
        case SDLK_KP_0:
          return '0';

        case SDLK_KP_PERIOD:
          return '.';

        case SDLK_KP_PLUS:
          return '+';

        case SDLK_KP_MINUS:
          return '-';

        case SDLK_KP_MULTIPLY:
          return '*';

        case SDLK_KP_DIVIDE:
          return '/';

        case SDLK_c:
          return TIB_CHAR_COS;

        case SDLK_s:
          return TIB_CHAR_SIN;

        case SDLK_t:
          return TIB_CHAR_TAN;
        }
    }

  for (unsigned int i = 0; i < (sizeof UNCHANGED_RANGES / sizeof (int)); ++i)
    if (code >= UNCHANGED_RANGES[i++] && code <= UNCHANGED_RANGES[i])
      return code;

  for (unsigned int i = 0; i < (sizeof UNCHANGED / sizeof (int)); ++i)
    if (code == UNCHANGED[i])
      return code;

  if (code >= SDLK_KP_1 && code <= SDLK_KP_9)
    return code - SDLK_KP_1 + '1';

  if (code >= SDLK_a && code <= SDLK_z)
    return code - SDLK_a + 'A';

  return 0;
}
