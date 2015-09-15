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

bool
lbt_Screen_get (const lbt_Screen *self, size_t x, size_t y)
{
  if (x >= self->width || y >= self->height)
    return false;

  return self->value[y][x];
}
