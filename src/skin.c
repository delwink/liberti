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
#include <string.h>

#include "log.h"
#include "skin.h"
#include "tibchar.h"
#include "tiberr.h"

#define DEFAULT_SKIN				\
  "screens=({"					\
  "mode=\"command\";"				\
  "x=0;"					\
  "y=0;"					\
  "});"

#define DEFAULT_SCREEN_WIDTH (96)
#define DEFAULT_SCREEN_HEIGHT (64)

#define foreachskin(L,E) for (E = L; E != NULL; E = E->next)

static PrefixTree *action_keywords = NULL;
static const SDL_Color BLACK = {0, 0, 0, 255};

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

  if (scale <= 0)
    return TIB_EBADFILE;

  if (NULL == node)
    {
      self->screens = malloc (sizeof (struct skin_screen_list));
      if (NULL == self->screens)
	return TIB_EALLOC;

      node = self->screens;
      self->active_screen = node;
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

  node->next = NULL;
  node->pos = pos;
  node->size.x = DEFAULT_SCREEN_WIDTH * scale;
  node->size.y = DEFAULT_SCREEN_HEIGHT * scale;
  lbt_Screen_set_mode (node->screen, mode);

  return 0;

 fail:
  if (prev)
    prev->next = NULL;
  lbt_Screen_decref (node->screen);
  self->active_screen = NULL;
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
  else if (0 == strcmp (s, "insert"))
    return TOGGLE_INSERT;

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

static void
free_render_cache (struct skin_render_cache *c)
{
  while (c)
    {
      struct skin_render_cache *temp = c;
      if (temp->surface)
	SDL_FreeSurface (temp->surface);

      c = c->next;
      free (temp);
    }
}

Skin *
open_skin (const char *path, lbt_State *state, struct point2d size)
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

  new->insert_mode = false;
  new->action_state = STATE_NORMAL;
  new->background = NULL;
  new->active_screen = NULL;
  new->buttons = NULL;
  new->renders = NULL;
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

      new->size.x = new->background->w;
      new->size.y = new->background->h;
    }
  else
    {
      new->size = size;
    }

  setting = config_lookup (&conf, "screens");
  if (setting)
    {
      if (!config_setting_is_list (setting))
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
	  pos.x = config_setting_get_int (setting);

	  setting = config_setting_get_member (screen, "y");
	  if (!setting || !config_setting_is_number (setting))
	    {
	      tib_errno = TIB_EBADFILE;
	      goto fail;
	    }

	  pos.y = config_setting_get_int (setting);

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

      if (--i)
	{
	  new->renders = malloc (sizeof (struct skin_render_cache));
	  if (!new->renders)
	    {
	      tib_errno = TIB_EALLOC;
	      goto fail;
	    }
	  new->renders->surface = NULL;

	  --i;
	}

      struct skin_render_cache *full = new->renders;
      for (; i > 0; --i, full = full->next)
	{
	  struct skin_render_cache *newfull;

	  newfull = malloc (sizeof (struct skin_render_cache));
	  if (!newfull)
	    {
	      tib_errno = TIB_EALLOC;
	      goto fail;
	    }
	  newfull->surface = NULL;

	  full->next = newfull;
	}
    }

  setting = config_lookup (&conf, "buttons");
  if (setting)
    {
      if (!config_setting_is_list (setting))
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
	  V = config_setting_get_int (setting);

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

  free_render_cache (new->renders);

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

  struct skin_button_list *b = self->buttons;
  while (b)
    {
      free (b->button);
      b = b->next;
      free (self->buttons);
      self->buttons = b;
    }

  free_render_cache (self->renders);
}

static bool
in_bounds (struct point2d start, struct point2d size, struct point2d pos)
{
  return pos.x >= start.x && pos.x <= (start.x + size.x)
    && pos.y >= start.y && pos.y <= (start.y + size.y);
}

static bool
on_skin (const Skin *self, struct point2d pos)
{
  int w = self->background->w, h = self->background->h;
  return in_bounds ((struct point2d) {0, 0}, (struct point2d) {w, h}, pos);
}

static bool
on_screen (const struct skin_screen_list *screen, struct point2d pos)
{
  return in_bounds (screen->pos, screen->size, pos);
}

static bool
on_button (const struct skin_button *button, struct point2d pos)
{
  return in_bounds (button->pos, button->size, pos);
}

static void
change_state (Skin *self, enum button_action_state state)
{
  if (self->action_state != state)
    self->action_state = state;
  else
    self->action_state = STATE_NORMAL;
}

static int
do_button_action (Skin *self, struct skin_button *button)
{
  enum lbt_screen_mode mode = self->active_screen->screen->mode;
  int x = 0, y = 0;
  lbt_Screen *screen = self->active_screen->screen;
  struct button_action_set action = button->actions[mode][self->action_state];
  union button_action which = action.which;

  switch (action.type)
    {
    case CHANGE_MODES:
      lbt_Screen_set_mode (screen, which.mode_open);
      break;

    case CHAR_INSERT:
      if (self->insert_mode)
	return lbt_Screen_insert_char (screen, which.char_insert);
      else
	return lbt_Screen_write_char (screen, which.char_insert);

    case CURSOR_MOVE:
      switch (which.cursor_move)
	{
	case UP:
	  x = 0;
	  y = 1;
	  break;

	case DOWN:
	  x = 0;
	  y = -1;
	  break;

	case LEFT:
	  x = -1;
	  y = 0;
	  break;

	case RIGHT:
	  x = 1;
	  y = 0;
	  break;
	}

      lbt_Screen_move_cursor (screen, x, y);
      break;

    case TOGGLE_2ND:
      change_state (self, STATE_2ND);
      break;

    case TOGGLE_ALPHA:
      change_state (self, STATE_ALPHA);
      break;

    case TOGGLE_INSERT:
      self->insert_mode = !self->insert_mode;
      break;
    }

  return 0;
}

int
Skin_click (Skin *self, struct point2d pos)
{
  if (!on_skin (self, pos))
    return TIB_EDIM;

  struct skin_screen_list *screen;
  foreachskin (self->screens, screen)
    {
      if (on_screen (screen, pos))
	{
	  if (screen != self->active_screen)
	    {
	      self->active_screen = screen;
	      self->action_state = STATE_NORMAL;
	    }

	  return 0;
	}
    }

  struct skin_button_list *bnode;
  foreachskin (self->buttons, bnode)
    {
      if (on_button (bnode->button, pos))
	return do_button_action (self, bnode->button);
    }

  return 0;
}

static SDL_Rect
get_rect (struct point2d pos, struct point2d size)
{
  SDL_Rect r =
    {
      .x = pos.x,
      .y = pos.y,
      .w = size.x,
      .h = size.y
    };

  return r;
}

static SDL_Surface *
render_line (const struct lbt_screen_line *line, TTF_Font *font)
{
  int rc, w, h, font_height;
  SDL_Surface **parts = malloc (tib_Expression_len (line->value)
				* sizeof (SDL_Surface *));
  if (!parts)
    {
      error ("Failed to initialize expression portions array");
      return NULL;
    }

  font_height = TTF_FontHeight (font);
  w = 0;
  h = font_height;

  size_t i;
  tib_foreachexpr (line->value, i)
    {
      int c = tib_Expression_ref (line->value, i);
      const char *s = tib_special_char_text (c);
      SDL_Surface *part = NULL;

      if (s)
	{
	  part = TTF_RenderUTF8_Solid (font, s, BLACK);
	}
      else
	{
	  char single[2];
	  single[0] = c;
	  single[1] = '\0';

	  part = TTF_RenderUTF8_Solid (font, single, BLACK);
	}

      if (!part)
	{
	  error ("Failed to render expression portion: %s", TTF_GetError ());
	  parts[i] = NULL;
	  continue;
	}

      w += part->w;
      if (w >= 96)
	{
	  h += font_height;
	  w -= part->w;
	}

      parts[i] = part;
    }

  SDL_Surface *final = SDL_CreateRGBSurface (0, w, h, 32, 0, 0, 0, 0);
  if (!final)
    {
      error ("Failed to initialize expression render surface: %s",
	     SDL_GetError ());
      goto fail;
    }

  rc = SDL_FillRect (final, NULL, SDL_MapRGBA (final->format, 0, 0, 0, 0));
  if (rc < 0)
    error ("Failed to set background of expression render surface: %s",
	   SDL_GetError ());

  w = 0;
  h = 0;
  for (i = 0; i < tib_Expression_len (line->value); ++i)
    {
      if (w + parts[i]->w >= 96)
	{
	  w = 0;
	  h += font_height;
	}

      SDL_Rect pos;
      pos.x = w;
      pos.y = h;

      rc = SDL_BlitSurface (parts[i], NULL, final, &pos);
      if (rc < 0)
	{
	  error ("Failed to blit expression portion: %s", SDL_GetError ());
	  continue;
	}

      w += parts[i]->w;
    }

 fail:
  for (i = 0; i < tib_Expression_len (line->value); ++i)
    if (parts[i])
      SDL_FreeSurface (parts[i]);

  free (parts);

  return final;
}

static SDL_Surface *
render_screen (lbt_Screen *screen, const struct fontset *fonts)
{
  int rc;
  SDL_Surface *final;

  final = SDL_CreateRGBSurface (0, 96, 64, 32, 0, 0, 0, 0);
  if (!final)
    {
      error ("Failed to initialize screen frame: %s", SDL_GetError ());
      return NULL;
    }

  rc = SDL_FillRect (final, NULL, SDL_MapRGB (final->format, 255, 255, 255));
  if (rc < 0)
    error ("Failed to fill screen frame with white background: %s",
	   SDL_GetError ());

  uint8_t height = 0;
  struct lbt_screen_line *line;
  switch (screen->mode)
    {
    case LBT_COMMAND_MODE:
      lbt_foreachline_rev (screen, line)
	{
	  SDL_Surface *line_render = render_line (line, fonts->reg);
	  if (line_render)
	    {
	      SDL_Rect pos;
	      pos.x = 0;
	      pos.y = 64 - height;

	      rc = SDL_BlitSurface (line_render, NULL, final, &pos);
	      if (rc < 0)
		error ("Failed to put line on screen frame: %s",
		       SDL_GetError ());

	      height += line_render->h;

	      SDL_FreeSurface (line_render);
	      if (height >= 64)
		break;
	    }
	}
      break;

    default:
      break;
    }

  return final;
}

SDL_Surface *
Skin_get_frame (Skin *self, const struct fontset *fonts)
{
  int rc;
  SDL_Surface *final;

  final = SDL_CreateRGBSurface (0, self->size.x, self->size.y, 32, 0, 0, 0, 0);
  if (!final)
    {
      error ("Failed to initialize skin frame: %s", SDL_GetError ());
      return NULL;
    }

  if (self->background)
    {
      rc = SDL_BlitSurface (self->background, NULL, final, NULL);
      if (rc < 0)
	error ("Failed to blit background: %s", SDL_GetError ());
    }

  struct skin_render_cache *full = self->renders;
  struct skin_screen_list *screen = self->screens;
  unsigned int i = 0;
  for (; screen != NULL; full = full->next, screen = screen->next, ++i)
    {
      SDL_Rect r = get_rect (screen->pos, screen->size);

      if (screen == self->active_screen)
	{
	  SDL_Surface *render = render_screen (screen->screen, fonts);
	  if (render)
	    {
	      if (full->surface)
		SDL_FreeSurface (full->surface);

	      full->surface = render;
	    }
	}

      if (full->surface)
	{
	  rc = SDL_BlitScaled (full->surface, NULL, final, &r);
	  if (rc < 0)
	    error ("Failed to blit screen %u: %s", i, SDL_GetError ());
	}
    }

  return final;
}
