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

#include <stdio.h>
#include <string.h>

#include "lbtstate.h"
#include "tiberr.h"

lbt_State *
lbt_new_State (const char *save_path)
{
  lbt_State *new = malloc (sizeof (lbt_State));
  if (NULL == new)
    {
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->save_path = malloc ((strlen (save_path) + 1) * sizeof (char));
  if (NULL == new->save_path)
    {
      free (new);
      tib_errno = TIB_EALLOC;
      return NULL;
    }

  new->refs = 1;
  strcpy (new->save_path, save_path);

  FILE *save = fopen (save_path, "r");
  if (NULL == save)
    {
      config_init (&new->conf);
    }
  else
    {
      int rc = config_read (&new->conf, save);
      fclose (save);
      if (!rc)
	{
	  config_destroy (&new->conf);
	  free (new);
	  tib_errno = TIB_EBADFILE;
	  return NULL;
	}
    }

  return new;
}

void
lbt_State_incref (lbt_State *self)
{
  ++self->refs;
}

void
lbt_State_decref (lbt_State *self)
{
  if (--self->refs == 0)
    {
      config_write_file (&self->conf, self->save_path);
      config_destroy (&self->conf);
      free (self->save_path);
      free (self);
    }
}
