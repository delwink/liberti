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

#include <ctype.h>
#include <libconfig.h>
#include <pfxtree.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <string.h>

#include "skin.h"
#include "tibchar.h"
#include "tiberr.h"

#define DEFAULT_SKIN				\
  "screens=[{"					\
  "mode=\"command\";"				\
  "x=0L;"					\
  "y=0L;"					\
  "}];"

#define foreachskin(L,E) for (E = L; E != NULL; E = E->next)

static PrefixTree *action_keywords = NULL;

static int
mode_from_string (const char *s)
{
  if (0 == strcmp (s, "command"))
    return LBT_COMMAND_MODE;

  return -1;
}

static int
add_screen (Skin *self, lbt_State *state, struct point2d pos,
	    enum lbt_screen_mode mode, double scale)
{
  struct skin_screen_list *node = self->screens, *prev = NULL;

  if (NULL == node)
    {
      self->screens = malloc (sizeof (struct skin_screen_list));
      if (NULL == self->screens)
	return TIB_EALLOC;

      node = self->screens;
    }
  else
    {
      while (node->next)
	node = node->next;

      node->next = malloc (sizeof (struct skin_screen_list));
      if (NULL == node->next)
	return TIB_EALLOC;

      prev = node;
      node = node->next;
    }

  node->screen = lbt_new_Screen (state);
  if (NULL == node->screen)
    goto fail;

  node->pos = pos;
  node->scale = scale;
  lbt_Screen_set_mode (node->screen, mode);

  return 0;

 fail:
  if (prev)
    prev->next = NULL;
  lbt_Screen_decref (node->screen);
  return tib_errno;
}

static char *
skin_file_path (const char *root, const char *file)
{
  size_t len = strlen (root) + strlen (file) + 2;
  char *full_path = malloc (len * sizeof (char));
  if (NULL == full_path)
    return NULL;

  sprintf (full_path, "%s/%s", root, file);
  return full_path;
}

static void
free_action_keywords ()
{
  pt_free (action_keywords);
  action_keywords = NULL;
}

static int
init_action_keywords ()
{
  int i;

  action_keywords = pt_new ();
  if (NULL == action_keywords)
    return TIB_EALLOC;

  for (i = TIB_CHAR_AND; i <= TIB_CHAR_YSCL; ++i)
    {
      const char *trans = tib_special_char_text (i);
      if (NULL == trans)
	continue;
      if (' ' == *trans)
	++trans; /* trim any leading space */

      char trim[11]; /* long enough to hold all translated strings and NUL */
      sprintf (trim, "%s", trans);

      char *p = strchr (trim, ' ');
      if (p)
	*p = '\0'; /* trim any trailing space */

      for (p = trim; *p != '\0'; ++p)
	*p = tolower (*p);

      tib_errno = pt_add (action_keywords, trim, i);
      if (tib_errno)
	goto fail;
    }

  return 0;

 fail:
  free_action_keywords ();
  return TIB_EALLOC;
}

static int
action_type_from_string (const char *s)
{
  if (NULL == s)
    return -1;

  if (0 == strcmp (s, "mode"))
    return CHANGE_MODES;
  else if (0 == strcmp (s, "char"))
    return CHAR_INSERT;
  else if (0 == strcmp (s, "move"))
    return CURSOR_MOVE;
  else if (0 == strcmp (s, "shift"))
    return TOGGLE_2ND;
  else if (0 == strcmp (s, "alpha"))
    return TOGGLE_ALPHA;

  return -1;
}

static bool
isdisplayable (char c)
{
  return (c > 31 && c < 96) || (c > 96 && c < 127);
}

static int
char_insert_from_string (const char *s)
{
  const PrefixTree *t = pt_search (action_keywords, s);
  if (t)
    return pt_data (t);

  if (strlen (s) != 1 || !isdisplayable (s[0]))
    return -1;

  return s[0];
}

static int
cursor_move_from_string (const char *s)
{
  if (NULL == s)
    return -1;

  if (0 == strcmp (s, "up"))
    return UP;
  else if (0 == strcmp (s, "down"))
    return DOWN;
  else if (0 == strcmp (s, "left"))
    return LEFT;
  else if (0 == strcmp (s, "right"))
    return RIGHT;

  return -1;
}

static int
get_action_set (const config_setting_t *mode, const char *name,
		struct button_action_set *action)
{
  const config_setting_t *root = config_setting_get_member (mode, name);
  if (!root)
    return TIB_EBADFILE;

  const config_setting_t *setting = config_setting_get_member (root, "type");
  if (!setting || CONFIG_TYPE_STRING != config_setting_type (setting))
    return TIB_EBADFILE;

  int i = action_type_from_string (config_setting_get_string (setting));
  if (-1 == i)
    return TIB_EBADFILE;

  action->type = i;
  setting = config_setting_get_member (root, "which");
  const char *value = NULL;
  if (setting)
    value = config_setting_get_string (setting);
  switch (i)
    {
    case CHANGE_MODES:
      if (!setting)
	return TIB_EBADFILE;

      i = mode_from_string (value);
      if (-1 == i)
	return TIB_EBADFILE;

      action->which.mode_open = i;
      break;

    case CHAR_INSERT:
      if (!setting)
	return TIB_EBADFILE;

      action->which.char_insert = char_insert_from_string (value);
      if (-1 == action->which.char_insert)
	return TIB_EBADFILE;
      break;

    case CURSOR_MOVE:
      if (!setting)
	return TIB_EBADFILE;

      i = cursor_move_from_string (value);
      if (-1 == i)
	return TIB_EBADFILE;

      action->which.cursor_move = i;
      break;

    default:
      break;
    }

  return 0;
}

static int
get_all_actions (const config_setting_t *mode,
		 struct button_action_set actions[])
{
  if (NULL == mode)
    return TIB_EBADFILE;

  tib_errno = init_action_keywords ();

#define GET_ACTION_SET(A,S) tib_errno = get_action_set (mode, S, &A);	\
  if (tib_errno) goto fail;

  GET_ACTION_SET (actions[STATE_NORMAL], "normal");
  GET_ACTION_SET (actions[STATE_2ND], "shift");
  GET_ACTION_SET (actions[STATE_ALPHA], "alpha");

 fail:
  free_action_keywords ();
  return tib_errno;
}

Skin *
open_skin (const char *path, lbt_State *state)
{
  if (NULL == state)
    {
      tib_errno = TIB_ENULLPTR;
      return NULL;
    }

  Skin *new = malloc (sizeof (Skin));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->background = NULL;
  new->active_screen = 0;
  new->buttons = NULL;
  new->full_renders = NULL;
  new->partial_renders = NULL;
  new->screens = NULL;

  config_t conf;
  config_init (&conf);

  if (path)
    {
      char *spec_path = skin_file_path (path, "spec.conf");
      if (NULL == spec_path)
	{
	  tib_errno = TIB_EALLOC;
	  goto fail;
	}

      tib_errno = config_read_file (&conf, spec_path);
      free (spec_path);
    }
  else
    {
      tib_errno = config_read_string (&conf, DEFAULT_SKIN);
    }

  if (CONFIG_FALSE == tib_errno)
    {
      tib_errno = TIB_EBADFILE;
      goto fail;
    }

  tib_errno = 0;

  config_setting_t *setting = config_lookup (&conf, "background");
  if (setting)
    {
      if (CONFIG_TYPE_STRING != config_setting_type (setting))
	{
	  tib_errno = TIB_EBADFILE;
	  goto fail;
	}

      char *bgpath = skin_file_path (path,
				     config_setting_get_string (setting));
      if (NULL == bgpath)
	{
	  tib_errno = TIB_EALLOC;
	  goto fail;
	}

      new->background = IMG_Load (bgpath);
      free (bgpath);
      if (NULL == new->background)
	{
	  tib_errno = TIB_EBADFILE;
	  goto fail;
	}
    }

  setting = config_lookup (&conf, "screens");
  if (setting)
    {
      if (!config_setting_is_array (setting))
	{
	  tib_errno = TIB_EBADFILE;
	  goto fail;
	}

      const config_setting_t *screens = setting;
      unsigned int i = 0;
      while ((setting = config_setting_get_elem (screens, i++)))
	{
	  if (!config_setting_is_group (setting))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  const config_setting_t *screen = setting;

	  setting = config_setting_get_member (screen, "mode");
	  if (!setting || config_setting_type (setting) != CONFIG_TYPE_STRING)
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  int mode = mode_from_string (config_setting_get_string (setting));
	  if (-1 == mode)
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  setting = config_setting_get_member (screen, "x");
	  if (!setting || !config_setting_is_number (setting))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  struct point2d pos;
	  pos.x = config_setting_get_int64 (setting);

	  setting = config_setting_get_member (screen, "y");
	  if (!setting || !config_setting_is_number (setting))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  pos.y = config_setting_get_int64 (setting);

	  double scale = 1.0;
	  setting = config_setting_get_member (screen, "scale");
	  if (setting)
	    {
	      if (!config_setting_is_number (setting))
		{
		  tib_errno = TIB_EBADFILE;
		  goto fail;
		}

	      scale = config_setting_get_float (setting);
	    }

	  tib_errno = add_screen (new, state, pos, mode, scale);
	  if (tib_errno)
	    goto fail;
	}
    }

  setting = config_lookup (&conf, "buttons");
  if (setting)
    {
      if (!config_setting_is_array (setting))
	{
	  tib_errno = TIB_EBADFILE;
	  goto fail;
	}

      new->buttons = malloc (sizeof (struct skin_button_list));
      if (NULL == new->buttons)
	{
	  tib_errno = TIB_EALLOC;
	  goto fail;
	}

      const config_setting_t *buttons = setting;
      struct skin_button_list *next = new->buttons;
      unsigned int i = 0;
      while ((setting = config_setting_get_elem (buttons, i++)))
	{
	  if (!config_setting_is_group (setting))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  if (i > 1)
	    {
	      next->next = malloc (sizeof (struct skin_button_list));
	      if (NULL == next->next)
		{
		  tib_errno = TIB_EALLOC;
		  goto fail;
		}

	      next = next->next;
	    }

	  next->button = malloc (sizeof (struct skin_button));
	  if (NULL == next->button)
	    {
	      tib_errno = TIB_EALLOC;
	      goto fail;
	    }

	  const config_setting_t *button = setting;

	  setting = config_setting_get_member (button, "actions");
	  if (!setting)
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  const config_setting_t *modes = setting;

	  setting = config_setting_get_member (modes, "default");
	  bool have_default = setting != NULL;
	  struct button_action_set default_actions[NUM_ACTION_STATES];
	  if (have_default)
	    {
	      tib_errno = get_all_actions (setting, default_actions);
	      if (tib_errno)
		goto fail;
	    }

	  size_t j;
	  struct button_action_set actions[NUM_ACTION_STATES];
#define ADD_ACTION(A,I) setting = config_setting_get_member (modes, A); \
	  if (setting)							\
	    {								\
	      tib_errno = get_all_actions (setting, actions);		\
	      if (tib_errno)						\
		goto fail;						\
	    }								\
	  else								\
	    {								\
	      for (j = 0; j < NUM_ACTION_STATES; ++j)			\
		actions[j] = default_actions[j];			\
	    }								\
	  for (j = 0; j < NUM_ACTION_STATES; ++j)			\
	    next->button->actions[I][j] = actions[j];

	  ADD_ACTION ("command", LBT_COMMAND_MODE);

#define ADD_DIM(D,V) setting = config_setting_get_member (button, D);	\
	  if (!setting || !config_setting_is_number (setting))		\
	    {								\
	      tib_errno = TIB_EBADFILE;					\
	      goto fail;						\
	    }								\
	  V = config_setting_get_int64 (setting);

	  ADD_DIM ("x", next->button->pos.x);
	  ADD_DIM ("y", next->button->pos.y);
	  ADD_DIM ("w", next->button->size.x);
	  ADD_DIM ("h", next->button->size.y);
	}
    }

  config_destroy (&conf);
  return new;

 fail:
  config_destroy (&conf);

  if (new->background)
    SDL_FreeSurface (new->background);

  if (new->screens)
    {
      struct skin_screen_list *s = new->screens;
      while (s)
	{
	  lbt_Screen_decref (s->screen);
	  s = s->next;
	  free (new->screens);
	  new->screens = s;
	}
    }

  if (new->buttons)
    {
      struct skin_button_list *s = new->buttons;
      while (s)
	{
	  if (s->button)
	    free (s->button);
	  s = s->next;
	  free (new->buttons);
	  new->buttons = s;
	}
    }

  free (new);
  return NULL;
}

static void
free_render_cache (struct skin_render_cache *c)
{
  while (c)
    {
      struct skin_render_cache *temp = c;
      SDL_FreeSurface (temp->surface);

      c = c->next;
      free (temp);
    }
}

void
free_skin (Skin *self)
{
  if (self->background)
    SDL_FreeSurface (self->background);

  struct skin_screen_list *s = self->screens;
  while (s)
    {
      lbt_Screen_decref (s->screen);
      s = s->next;
      free (self->screens);
      self->screens = s;
    }

  self->active_screen = 0;

  struct skin_button_list *b = self->buttons;
  while (b)
    {
      free (b->button);
      b = b->next;
      free (self->buttons);
      self->buttons = b;
    }

  free_render_cache (self->full_renders);
  self->full_renders = NULL;

  if (self->partial_renders)
    {
      struct skin_screen_render_cache *c = self->partial_renders;
      while (c)
	{
	  struct skin_screen_render_cache *temp = c;
	  free_render_cache (c->renders);

	  c = c->next;
	  free (temp);
	}
    }
}
