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

lbt_Screen *
lbt_new_Screen (size_t width, size_t height)
{
  lbt_Screen *new = malloc (sizeof (lbt_Screen));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->refs = 1;
  new->height = height;
  new->width = width;
  new->mode = LBT_COMMAND_MODE;
  new->lines = NULL;

  new->value = malloc (height * sizeof (bool *));
  if (NULL == new->value)
    {
      free (new);
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  size_t i, j;
  for (i = 0; i < height; ++i)
    {
      new->value[i] = malloc (width * sizeof (bool));
      if (NULL == new->value[i])
	{
	  for (j = 0; j < i; ++j)
	    free (new->value[j]);
	  free (new->value);
	  tib_errno = TIB_EALLOC;
	  return NULL;
	}

      for (j = 0; j < width; ++j)
	new->value[i][j] = false;
    }

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
  if (--self->refs == 0)
    {
      size_t i;
      for (i = 0; i < self->height; ++i)
	free (self->value[i]);
      free (self->value);

      lbt_Screen_clear_lines (self);

      free (self);
    }
}

void
lbt_Screen_clear (lbt_Screen *self)
{
  size_t i, j;
  for (i = 0; i < self->height; ++i)
    for (j = 0; j < self->width; ++j)
      self->value[i][j] = false;
}

size_t
lbt_Screen_height (const lbt_Screen *self)
{
  return self->height;
}

size_t
lbt_Screen_width (const lbt_Screen *self)
{
  return self->width;
}

int
lbt_Screen_set (lbt_Screen *self, size_t x, size_t y, bool state)
{
  if (x >= self->width || y >= self->height)
    return TIB_EINDEX;

  self->value[y][x] = state;
  return 0;
}

int
lbt_Screen_toggle (lbt_Screen *self, size_t x, size_t y)
{
  return lbt_Screen_set (self, x, y, !lbt_Screen_get (self, x, y));
}

bool
lbt_Screen_get (const lbt_Screen *self, size_t x, size_t y)
{
  if (x >= self->width || y >= self->height)
    return false;

  return self->value[y][x];
}

static struct lbt_screen_line *
new_line (const tib_Expression *text, size_t x, size_t y)
{
  struct lbt_screen_line *new = malloc (sizeof (struct lbt_screen_line));
  if (NULL == new)
    return NULL;

  new->value = tib_copy_Expression (text);
  if (NULL == new->value)
    {
      free (new);
      return NULL;
    }

  new->x = x;
  new->y = y;
  new->next = NULL;

  return new;
}

int
lbt_Screen_add_line (lbt_Screen *self, const tib_Expression *text, size_t x,
		     size_t y)
{
  if (!self->lines)
    {
      self->lines = new_line (text, x, y);
      if (NULL == self->lines)
	return TIB_EALLOC;

      return 0;
    }

  struct lbt_screen_line *last;
  lbt_foreachline (self, last)
    ; /* set to the last line in the list */

  last->next = new_line (text, x, y);
  if (NULL == last->next)
    return TIB_EALLOC;

  return 0;
}

struct lbt_screen_line *
lbt_Screen_get_line (const lbt_Screen *self, size_t i)
{
  struct lbt_screen_line *line;
  lbt_foreachline (self, line)
    {
      if (0 == i)
	return line;

      --i;
    }

  return NULL;
}

void
lbt_Screen_del_line (lbt_Screen *self, size_t i)
{
  struct lbt_screen_line *temp, *before = lbt_Screen_get_line (self, i-1);

  if (NULL == before)
    {
      if (0 == i && self->lines)
	{
	  temp = self->lines;
	  self->lines = temp->next;
	}
      else
	{
	  return;
	}
    }
  else if (NULL == before->next)
    {
      return;
    }
  else
    {
      temp = before->next;
      before->next = temp->next;
    }

  tib_Expression_decref (temp->value);
  free (temp);
}

void
lbt_Screen_clear_lines (lbt_Screen *self)
{
  struct lbt_screen_line *line;
  while ((line = self->lines))
    lbt_Screen_del_line (self, 0);
}

int
lbt_Screen_set_mode (lbt_Screen *self, enum lbt_screen_mode mode)
{
  lbt_Screen_clear (self);
  lbt_Screen_clear_lines (self);

  self->mode = mode;

  switch (mode) /* TODO: initialize mode */
    {

    default:
      return 0;
    }
}

enum lbt_screen_mode
lbt_Screen_get_mode (const lbt_Screen *self)
{
  return self->mode;
}
