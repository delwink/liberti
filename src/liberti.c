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

#include <errno.h>
#include <gsl/gsl_errno.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <wordexp.h>

#include "font.h"
#include "log.h"
#include "skin.h"
#include "tibchar.h"
#include "tibfunction.h"
#include "tibvar.h"

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

#ifdef RUN_IN_PLACE
# define USER_DIR "."
#else
# define USER_DIR "~/.liberti"
#endif

#define USER_STATE_PATH "state.conf"

static Uint32
timer (Uint32 interval, void *data)
{
  SDL_Event event;
  SDL_UserEvent user;

  user.type = SDL_USEREVENT;
  user.code = 0;
  user.data1 = data;

  event.type = SDL_USEREVENT;
  event.user = user;

  SDL_PushEvent (&event);
  return interval;
}

static char *
get_conf_path (const char *path)
{
  wordexp_t w;
  int rc = wordexp (USER_DIR, &w, 0);
  if (rc)
    return NULL;

  size_t len = strlen (w.we_wordv[0]) + strlen (path) + 2;
  char *out = malloc (len * sizeof (char));
  if (out)
    snprintf (out, len, "%s/%s", w.we_wordv[0], path);

  wordfree (&w);
  return out;
}

int
main (int argc, char *argv[])
{
  bool user_dir_exists = false, state_file_exists = false, state_init = false;
  int rc = 0;
  SDL_TimerID timer_id = 0;
  SDL_Window *window = NULL;
  Skin *skin = NULL;

  char *skin_path = NULL, *state_path = NULL;
  Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

  struct option longopts[] = 
    {
      {"debug",      no_argument,       0, 'd'},
      {"fullscreen", no_argument,       0, 'f'},
      {"help",       no_argument,       0, 'h'},
      {"skin",       required_argument, 0, 's'},
      {"version",    no_argument,       0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "dfhs:v", longopts, &longindex))
             != -1)
        {
          switch (c)
            {
            case 'd':
              debug_mode = true;
              break;

            case 'f':
              window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
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

  gsl_set_error_handler_off ();

  rc = SDL_Init (SDL_INIT_TIMER);
  if (rc)
    {
      critical ("Could not initialize SDL timer: %s", SDL_GetError ());
      goto end;
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

  rc = font_init ();
  if (rc)
    goto end;

  rc = tib_keyword_init ();
  if (rc)
    {
      critical ("Could not initialize TI-BASIC keyword lookup tree: Error %d",
                rc);
      goto end;
    }

  rc = tib_var_init ();
  if (rc)
    {
      critical ("Could not initialize default variables: Error %d", rc);
      goto end;
    }

  rc = tib_registry_init ();
  if (rc)
    {
      critical ("Could not initialize function registry: Error %d", rc);
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

  {
    char *user_dir_path = get_conf_path ("");
    if (user_dir_path)
      {
        rc = mkdir (user_dir_path, 0755);
        free (user_dir_path);
        if (rc && EEXIST != errno)
          {
            warn ("Failed to create config directory: %s", strerror (errno));
          }
        else
          {
            user_dir_exists = true;

            state_path = get_conf_path (USER_STATE_PATH);
            if (state_path)
              {
                rc = access (state_path, F_OK);
                if (rc)
                  {
                    debug ("Can't open state file: %s", strerror (errno));
                    info ("Loading empty state");
                  }
                else
                  {
                    state_file_exists = true;
                    info ("Loading state from %s", state_path);
                  }
              }
          }
      }
    else
      {
        warn ("Could not determine config path");
      }
  }

  struct state state;
  rc = load_state (&state, state_file_exists ? state_path : NULL);
  if (rc)
    {
      critical ("Could not initialize calculator state. Error %d", rc);
      goto end;
    }

  state_init = true;

  timer_id = SDL_AddTimer (500, timer, &state);
  if (!timer_id)
    {
      critical ("Could not add blink timer: %s", SDL_GetError ());
      goto end;
    }

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
                             window_flags);
  if (!window)
    {
      critical ("Could not create window: %s", SDL_GetError ());
      goto end;
    }

  for (;;)
    {
      SDL_Surface *frame = Skin_get_frame (skin);
      if (frame)
        {
          SDL_Surface *screen = SDL_GetWindowSurface (window);
          SDL_BlitScaled (frame, NULL, screen, NULL);
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

            case SDL_USEREVENT:
              switch (event.user.code)
                {
                case 0:
                  {
                    struct state *state = event.user.data1;
                    state->blink_state = !state->blink_state;
                  }
                  break;

                default:
                  break;
                }
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
  if (timer_id)
    SDL_RemoveTimer (timer_id);

  SDL_Quit ();
  SDL_VideoQuit ();
  IMG_Quit ();

  font_free ();
  tib_keyword_free ();
  tib_registry_free ();
  tib_var_free ();

  if (state_init)
    {
      if (user_dir_exists && state_path)
        {
          errno = save_state (&state, state_path);
          if (errno)
            warn ("Failed to save state");
        }

      state_destroy (&state);
    }

  if (state_path)
    free (state_path);

  if (skin)
    free_skin (skin);

  if (window)
    SDL_DestroyWindow (window);

  return !!rc;
}
