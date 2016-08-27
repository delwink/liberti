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
#include "util.h"

static void
add_history (struct state *state, struct tib_expr *in, struct tib_expr *ans_s,
             TIB *ans)
{
  if (MAX_HISTORY == state->history_len)
    {
      tib_expr_destroy (&state->history[0].entry);
      tib_expr_destroy (&state->history[0].answer_string);
      tib_decref (state->history[0].answer);

      for (unsigned int i = 0; i < MAX_HISTORY - 1; ++i)
        state->history[i] = state->history[i + 1];
    }
  else
    {
      ++state->history_len;
    }

  unsigned int i = state->history_len - 1;
  state->history[i].entry = *in;
  state->history[i].answer_string = *ans_s;
  state->history[i].answer = ans;
}

int
load_state (struct state *dest, const char *path)
{
  int rc = 0;

  if (!dest)
    return TIB_ENULLPTR;

  dest->action_state = STATE_NORMAL;
  dest->entry_cursor = 0;
  dest->history_len = 0;
  dest->blink_state = true;
  dest->insert_mode = false;

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
          rc = tib_encode_str (&dest->entry, s);
          if (rc)
            goto fail;

          rc = state_calc_entry (dest);
          if (rc)
            goto fail;
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

        char *s = tib_expr_tostr (&state->history[i].entry);
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
  tib_expr_destroy (&state->entry);
  state_clear_history (state);
}

void
entry_move_cursor (struct state *state, int distance)
{
  state->entry_cursor += distance;

  if (state->entry_cursor > state->entry.len)
    state->entry_cursor = state->entry.len;
  else if (state->entry_cursor < 0)
    state->entry_cursor = 0;

  state->action_state = STATE_NORMAL;
}

static int
entry_insert (struct state *state, int c)
{
  int rc = tib_expr_insert (&state->entry, state->entry_cursor, c);
  if (!rc)
    {
      ++state->entry_cursor;
      state->action_state = STATE_NORMAL;
      state->insert_mode = false;
    }

  return rc;
}

int
entry_write (struct state *state, int c)
{
  if (state->insert_mode || state->entry_cursor == state->entry.len)
    return entry_insert (state, c);

  state->entry.data[state->entry_cursor++] = c;
  state->action_state = STATE_NORMAL;
  return 0;
}

int
entry_recall (struct state *state)
{
  if (state->history_len)
    {
      int rc = tib_exprcpy (&state->entry,
                            &state->history[state->history_len - 1].entry);
      if (rc)
        return rc;

      state->entry_cursor = state->entry.len;
    }
  else
    {
      state->entry.len = 0;
    }

  return 0;
}

void
change_action_state (struct state *state, enum action_state action_state)
{
  if (state->action_state != action_state)
    state->action_state = action_state;
  else
    state->action_state = STATE_NORMAL;
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
  struct tib_expr in_copy = { .bufsize = 0 };
  int rc = tib_exprcpy (&in_copy, in);
  if (rc)
    return rc;

  TIB *ans_copy = tib_copy (answer);
  if (!ans_copy)
    {
      tib_expr_destroy (&in_copy);
      return tib_errno;
    }

  struct tib_expr ans_s;
  rc = tib_toexpr (&ans_s, ans_copy);
  if (rc)
    {
      rc = load_expr (&ans_s, "Error");
      if (rc)
        {
          tib_expr_destroy (&in_copy);
          tib_decref (ans_copy);
          return rc;
        }
    }

  add_history (state, &in_copy, &ans_s, ans_copy);
  return 0;
}

void
state_clear_history (struct state *state)
{
  for (unsigned int i = 0; i < state->history_len; ++i)
    {
      tib_expr_destroy (&state->history[i].entry);
      tib_expr_destroy (&state->history[i].answer_string);
      tib_decref (state->history[i].answer);
    }

  state->history_len = 0;
}
