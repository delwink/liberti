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
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include "log.h"
#include "skin.h"
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
  bool state_init = false;
  int rc = 0;
  SDL_Window *window = NULL;
  Skin *skin = NULL;
  struct fontset *fonts = NULL;

  const char *skin_path = NULL;

  struct option longopts[] = 
    {
      {"debug",   no_argument,       0, 'd'},
      {"help",    no_argument,       0, 'h'},
      {"skin",    required_argument, 0, 's'},
      {"version", no_argument,       0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "dhs:v", longopts, &longindex))
             != -1)
        {
          switch (c)
            {
            case 'd':
              debug_mode = true;
              break;

            case 'h':
              puts (USAGE_INFO);
              return 0;

            case 's':
              skin_path = optarg;
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

  SDL_DisplayMode display_mode;
  rc = SDL_GetCurrentDisplayMode (0, &display_mode);
  if (rc)
    {
      critical ("Could not get screen information: %s", SDL_GetError ());
      goto end;
    }

  debug ("Screen resolution: %dx%d", display_mode.w, display_mode.h);

  struct state state;
  rc = load_state (&state, NULL);
  if (rc)
    {
      critical ("Could not initialize calculator state. Error %d", rc);
      goto end;
    }

  state_init = true;

  skin = open_skin (skin_path, &state, (struct point2d) { 96, 64 });
  if (!skin)
    {
      critical ("Could not load skin");
      goto end;
    }

  window = SDL_CreateWindow ("LiberTI " VERSION_STRING,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             skin->size.x,
                             skin->size.y,
                             SDL_WINDOW_OPENGL);
  if (!window)
    {
      critical ("Could not create window: %s", SDL_GetError ());
      goto end;
    }

  for (;;)
    {
      SDL_Surface *frame = Skin_get_frame (skin, fonts);
      if (frame)
        {
          SDL_Surface *screen = SDL_GetWindowSurface (window);
          SDL_BlitSurface (frame, NULL, screen, NULL);
          SDL_FreeSurface (frame);
          SDL_UpdateWindowSurface (window);
        }

      SDL_Event event;
      SDL_WaitEvent (NULL);

      while (SDL_PollEvent (&event))
        {
          switch (event.type)
            {
            case SDL_KEYDOWN:
              rc = Skin_input (skin, &event.key);
              if (rc)
                error ("Error %d processing key entry", rc);
              break;

            case SDL_QUIT:
              info ("Got SDL quit event");
              goto end;

            default:
              break;
            }
        }
    }

 end:
  SDL_VideoQuit ();
  IMG_Quit ();

  if (skin)
    free_skin (skin);

  if (state_init)
    state_destroy (&state);

  if (fonts)
    free_font_set (fonts);

  if (window)
    SDL_DestroyWindow (window);

  return rc;
}
