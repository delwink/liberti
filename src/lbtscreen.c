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

#include "lbtcharmap.h"
#include "lbtscreen.h"
#include "tiberr.h"
#include "tibexpr.h"

static bool **screen = NULL;
static struct lbt_screen_dim d;

int
lbt_screen_init (struct lbt_screen_dim dims)
{
  if (screen != NULL)
    lbt_screen_free ();

  screen = malloc (dims.height * sizeof (bool *));
  if (NULL == screen)
    return TIB_EALLOC;

  size_t i, j;
  for (i = 0; i < dims.height; ++i)
    {
      screen[i] = malloc (dims.width * sizeof (bool));
      if (NULL == screen[i])
	goto fail;

      for (j = 0; j < dims.width; ++j)
	screen[i][j] = false;
    }

  d = dims;

  return 0;

 fail:
  for (j = 0; j < i; ++j)
    free (screen[j]);
  free (screen);
  screen = NULL;

  return TIB_EALLOC;
}

void
lbt_screen_free ()
{
  if (screen)
    {
      size_t i;
      for (i = 0; i < d.height; ++i)
	free (screen[i]);
      free (screen);

      screen = NULL;
    }
}

void
lbt_clear_screen ()
{
  size_t i, j;
  for (i = 0; i < d.height; ++i)
    for (j = 0; j < d.width; ++j)
      screen[i][j] = false;
}

int
lbt_set_pixel (size_t x, size_t y, bool state)
{
  if (x >= d.width || y >= d.height)
    return TIB_EINDEX;

  screen[y][x] = state;
  return 0;
}

bool
lbt_get_pixel (size_t x, size_t y)
{
  if (x >= d.width || y >= d.height)
    return false;

  return screen[y][x];
}
