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

#include <SDL.h>
#include <unistd.h>
#include <getopt.h>

#include "log.h"

#define VERSION_INFO "LiberTI 0.0.0\n\
Copyright (C) 2015 Delwink, LLC\n\
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>.\n\
This is libre software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by David McMackins II."

int
main (int argc, char *argv[])
{
  int rc = 0;
  SDL_DisplayMode display_mode;

  struct option longopts[] = 
    {
      {"debug", no_argument, 0, 'd'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "dv", longopts, &longindex)) != -1)
	{
	  switch (c)
	    {
	    case 'd':
	      debug_mode = true;
	      break;

	    case 'v':
	      puts (VERSION_INFO);
	      return 0;

	    case '?':
	      return 1;
	    }
	}
    }

  rc = SDL_VideoInit (NULL);
  if (rc)
    {
      critical ("Could not initialize SDL video: %s", SDL_GetError ());
      goto end;
    }

  rc = SDL_GetCurrentDisplayMode (0, &display_mode);
  if (rc)
    {
      critical ("Could not get screen information: %s", SDL_GetError ());
      goto end;
    }

  debug ("Screen resolution: %dx%d", display_mode.w, display_mode.h);

 end:
  SDL_VideoQuit ();
  return rc;
}
