/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2015-2016 Delwink, LLC
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
#include <SDL_image.h>
#include <unistd.h>
#include <getopt.h>

#include "log.h"
#include "skin.h"
#include "state.h"
#include "ttf.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
# define VERSION_STRING PACKAGE_VERSION
# define PROG PACKAGE
#else
# define VERSION_STRING "custom build"
# define PROG "liberti"
#endif

#define USAGE_INFO "USAGE: " PROG " [options]\n\n\
OPTIONS:\n\
\t-d, --debug\tPrints extra activity while running\n\
\t-h, --help\tPrints this help message and exits\n\
\t-v, --version\tPrints version info and exits\n"

#define VERSION_INFO "LiberTI " VERSION_STRING "\n\
Copyright (C) 2015-2016 Delwink, LLC\n\
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>.\n\
This is libre software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by David McMackins II."

int
main (int argc, char *argv[])
{
  int rc = 0;
  SDL_DisplayMode display_mode;
  struct fontset *fonts = NULL;

  struct option longopts[] = 
    {
      {"debug",   no_argument, 0, 'd'},
      {"help",    no_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "dhv", longopts, &longindex)) != -1)
        {
          switch (c)
            {
            case 'd':
              debug_mode = true;
              break;

            case 'h':
              puts (USAGE_INFO);
              return 0;

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

  rc = IMG_Init (IMG_INIT_PNG);
  if (rc != IMG_INIT_PNG)
    {
      critical ("Could not initialize PNG image library: %s", IMG_GetError ());
      rc = 1;
      goto end;
    }

  fonts = get_font_set (7);
  if (!fonts)
    {
      critical ("Could not load fonts: %s", TTF_GetError ());
      rc = 1;
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
  IMG_Quit ();

  if (fonts)
    free_font_set (fonts);

  return rc;
}
