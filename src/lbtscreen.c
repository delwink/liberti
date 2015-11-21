/*
 *  libliberti - Backend functionality for LiberTI
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

#include "lbtscreen.h"
#include "tiberr.h"

#define LINES self->state->lines[self->mode]

lbt_Screen *
lbt_new_Screen (lbt_State *state)
{
  lbt_Screen *new = malloc (sizeof (lbt_Screen));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->refs = 1;
  new->mode = LBT_COMMAND_MODE;

  lbt_State_incref (state);
  new->state = state;

  return new;
}

void
lbt_Screen_incref (lbt_Screen *self)
{
  ++self->refs;
}

void
lbt_Screen_decref (lbt_Screen *self)
{
  if (0 == self->refs)
    return;

  if (--self->refs == 0)
    {
      lbt_State_decref (self->state);
      free (self);
    }
}

int
lbt_Screen_add_line (lbt_Screen *self, const tib_Expression *text, int64_t x,
		     int64_t y)
{
  return lbt_State_add_line (self->state, text, x, y, self->mode);
}

struct lbt_screen_line *
lbt_Screen_get_line (const lbt_Screen *self, size_t i)
{
  return lbt_State_get_line (self->state, i, self->mode);
}

void
lbt_Screen_del_line (lbt_Screen *self, size_t i)
{
  lbt_State_del_line (self->state, i, self->mode);
}

void
lbt_Screen_clear_lines (lbt_Screen *self)
{
  lbt_State_clear_lines (self->state, self->mode);
}

size_t
lbt_Screen_num_lines (const lbt_Screen *self)
{
  return lbt_State_num_lines (self->state, self->mode);
}

void
lbt_Screen_set_state (lbt_Screen *self, lbt_State *state)
{
  /* increase ref count of this one first to prevent freeing if it's the
     same state */
  lbt_State_incref (state);

  lbt_State_decref (self->state);
  self->state = state;
}

void
lbt_Screen_refresh (lbt_Screen *self)
{
  lbt_Screen_set_mode (self, self->mode);
}

int64_t
to_bounds (int64_t x, int64_t min, int64_t max)
{
  if (x < min)
    x = min;
  else if (x > max)
    x = max;

  return x;
}

static struct lbt_screen_line *
current_line (lbt_Screen *self)
{
  switch (self->mode)
    {
    case LBT_COMMAND_MODE:
      return lbt_Screen_get_line (self, self->cursor.y);

    default:
      return NULL;
    }
}

void
lbt_Screen_move_cursor (lbt_Screen *self, int64_t x, int64_t y)
{
  self->cursor.x += x;
  self->cursor.y += y;

  switch (self->mode)
    {
    case LBT_COMMAND_MODE:
      self->cursor.y = to_bounds (self->cursor.y, 0,
				  (int64_t) lbt_Screen_num_lines (self));

      struct lbt_screen_line *line = current_line (self);
      if (line)
	self->cursor.x = to_bounds (self->cursor.x, 0,
				    tib_Expression_len (line->value));
      break;

    default:
      break;
    }
}

int
lbt_Screen_write_char (lbt_Screen *self, int c)
{
  struct lbt_screen_line *line = current_line (self);

  int64_t x = self->cursor.x;
  if (NULL == line || x < 0 || (size_t) x > tib_Expression_len (line->value))
    return TIB_EINDEX;

  int rc = 0;
  if (tib_Expression_len (line->value) == (size_t) x)
    rc = tib_Expression_push (line->value, c);
  else
    line->value->value[x] = c;

  if (!rc)
    ++self->cursor.x;
  return rc;
}

int
lbt_Screen_insert_char (lbt_Screen *self, int c)
{
  struct lbt_screen_line *line = current_line (self);
  if (NULL == line)
    return TIB_EINDEX;

  int rc = tib_Expression_insert (line->value, (size_t) self->cursor.x, c);
  if (!rc)
    ++self->cursor.x;

  return rc;
}

void
lbt_Screen_set_mode (lbt_Screen *self, enum lbt_screen_mode mode)
{
  switch (mode)
    {
    case LBT_COMMAND_MODE:
      self->cursor.x = 0;
      self->cursor.y = (int64_t) lbt_Screen_num_lines (self);
      break;

    default:
      return;
    }

  self->mode = mode;
}

enum lbt_screen_mode
lbt_Screen_get_mode (const lbt_Screen *self)
{
  return self->mode;
}
