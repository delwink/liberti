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

#include <libconfig.h>
#include <stdlib.h>

#include "state.h"
#include "tibchar.h"
#include "tiberr.h"
#include "tibeval.h"

static int
load_line (struct tib_expr *expr, const char *s)
{
  int rc = tib_expr_init (expr);
  if (rc)
    return rc;

  rc = tib_encode_str (expr, s);
  return rc;
}

static void
add_history (struct state *state, struct tib_expr *in, struct tib_expr *ans_s,
	     TIB *ans)
{
  if (MAX_HISTORY == state->history_len)
    {
      tib_expr_free_data (&state->history[0]);
      tib_expr_free_data (&state->answer_strings[0]);
      tib_decref (state->answers[0]);

      for (unsigned int i = 0; i < MAX_HISTORY - 1; ++i)
	{
	  state->history[i] = state->history[i + 1];
	  state->answer_strings[i] = state->answer_strings[i + 1];
	  state->answers[i] = state->answers[i + 1];
	}
    }
  else
    {
      ++state->history_len;
    }

  unsigned int i = state->history_len - 1;
  state->history[i] = *in;
  state->answer_strings[i] = *ans_s;
  state->answers[i] = ans;
}

int
load_state (struct state *dest, const char *path)
{
  int rc = 0;

  if (!dest)
    return TIB_ENULLPTR;

  dest->entry_cursor = 0;
  dest->history_len = 0;

  rc = tib_expr_init (&dest->entry);
  if (rc)
    return rc;

  if (!path)
    return 0;

  config_t conf;
  config_init (&conf);

  rc = config_read_file (&conf, path);
  if (CONFIG_FALSE == rc)
    {
      config_destroy (&conf);
      return TIB_EBADFILE;
    }

  config_setting_t *setting = config_lookup (&conf, "history");
  if (setting)
    {
      if (config_setting_type (setting) != CONFIG_TYPE_LIST)
	{
	  rc = TIB_EBADFILE;
	  goto fail;
	}

      unsigned int i = 0;
      config_setting_t *line;

      while ((line = config_setting_get_elem (setting, i++)))
	{
	  if (config_setting_type (line) != CONFIG_TYPE_GROUP)
	    {
	      rc = TIB_EBADFILE;
	      goto fail;
	    }

	  config_setting_t *e = config_setting_get_member (line, "input");
	  if (!e || config_setting_type (e) != CONFIG_TYPE_STRING)
	    {
	      rc = TIB_EBADFILE;
	      goto fail;
	    }

	  const char *s = config_setting_get_string (e);
	  struct tib_expr hist;
	  rc = load_line (&hist, s);
	  if (rc)
	    goto fail;

	  e = config_setting_get_member (line, "output");
	  if (!e || config_setting_type (e) != CONFIG_TYPE_STRING)
	    {
	      rc = TIB_EBADFILE;
	      goto fail;
	    }

	  s = config_setting_get_string (e);
	  struct tib_expr ans_s;
	  rc = load_line (&ans_s, s);
	  if (rc)
	    {
	      tib_expr_free_data (&hist);
	      goto fail;
	    }

	  TIB *ans = tib_eval (&ans_s);
	  if (!ans)
	    {
	      rc = tib_errno;
	      tib_expr_free_data (&hist);
	      tib_expr_free_data (&ans_s);
	      goto fail;
	    }

	  add_history (dest, &hist, &ans_s, ans);
	}
    }

  config_destroy (&conf);
  return 0;

 fail:
  state_destroy (dest);
  config_destroy (&conf);
  return rc;
}

int
save_state (const struct state *state, const char *path)
{
  int rc = 0;

  if (!state || !path)
    return TIB_ENULLPTR;

  config_t conf;
  config_init (&conf);

  config_setting_t * const root = config_root_setting (&conf);

#define CHECK_NULL(P) if (!(P)) { rc = TIB_EALLOC; goto end; }

  {
    config_setting_t *history = config_setting_add (root, "history",
						    CONFIG_TYPE_LIST);
    CHECK_NULL (history);

    for (unsigned int i = 0; i < state->history_len; ++i)
      {
	config_setting_t * const line = config_setting_add (history, NULL,
							    CONFIG_TYPE_GROUP);
	CHECK_NULL (line);

	config_setting_t *info = config_setting_add (line, "input",
						     CONFIG_TYPE_STRING);
	CHECK_NULL (info);

	char *s = tib_expr_tostr (&state->history[i]);
	if (!s)
	  {
	    rc = tib_errno;
	    goto end;
	  }

	rc = config_setting_set_string (info, s);
	free (s);
	if (CONFIG_FALSE == rc)
	  {
	    rc = TIB_EALLOC;
	    goto end;
	  }

	info = config_setting_add (line, "output", CONFIG_TYPE_STRING);
	CHECK_NULL (info);

	s = tib_expr_tostr (&state->answer_strings[i]);
	if (!s)
	  {
	    rc = tib_errno;
	    goto end;
	  }

	rc = config_setting_set_string (info, s);
	free (s);
	if (CONFIG_FALSE == rc)
	  {
	    rc = TIB_EALLOC;
	    goto end;
	  }
      }
  }

#undef CHECK_NULL

  rc = 0;
  config_write_file (&conf, path);

 end:
  config_destroy (&conf);
  return rc;
}

void
state_destroy (struct state *state)
{
  tib_expr_free_data (&state->entry);
  state_clear_history (state);
}

void
entry_move_cursor (struct state *state, int distance)
{
  if (distance < 0)
    {
      distance *= -1;

      if (state->entry_cursor > (unsigned int) distance)
	state->entry_cursor -= distance;
      else
	state->entry_cursor = 0;
    }
  else
    {
      state->entry_cursor += distance;

      if (state->entry_cursor > state->entry.len)
	state->entry_cursor = state->entry.len;
    }
}

int
entry_insert (struct state *state, int c)
{
  int rc = tib_expr_insert (&state->entry, state->entry_cursor, c);
  if (!rc)
    ++state->entry_cursor;

  return rc;
}

int
entry_write (struct state *state, int c)
{
  if (state->entry_cursor == state->entry.len)
    return entry_insert (state, c);

  state->entry.data[state->entry_cursor++] = c;
  return 0;
}

int
state_calc_entry (struct state *state)
{
  TIB *ans = tib_eval (&state->entry);
  if (!ans)
    return tib_errno;

  int rc = state_add_history (state, &state->entry, ans);
  tib_decref (ans);
  if (rc)
    return rc;

  state->entry_cursor = 0;
  state->entry.len = 0;
  return 0;
}

int
state_add_history (struct state *state, const struct tib_expr *in,
		   const TIB *answer)
{
  struct tib_expr in_copy;
  int rc = tib_exprcpy (&in_copy, in);
  if (rc)
    return rc;

  TIB *ans_copy = tib_copy (answer);
  if (!ans_copy)
    {
      tib_expr_free_data (&in_copy);
      return tib_errno;
    }

  struct tib_expr ans_s;

  // TODO: create function for converting TIB into expression

  add_history (state, &in_copy, &ans_s, ans_copy);
  return 0;
}

void
state_clear_history (struct state *state)
{
  for (unsigned int i = 0; i < state->history_len; ++i)
    {
      tib_expr_free_data (&state->history[i]);
      tib_expr_free_data (&state->answer_strings[i]);
      tib_decref (state->answers[i]);
    }

  state->history_len = 0;
}
