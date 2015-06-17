/*
 *  tibdecode - Decompile a TI BASIC program
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

#include "tibtranscode.h"
#include "tiberr.h"

int
main (int argc, char *argv[])
{
  FILE *out = stdout;
  char *inpath = NULL;
  size_t parsed;

  if (2 == argc)
    inpath = argv[1];

  tib_Expression *translated = tib_fread (inpath, &parsed);
  if (NULL == translated)
    {
      fprintf (stderr, "tibdecode: Error %d occurred while processing. "
	       "Parsed %lu characters.\n",
	       tib_errno, parsed);
      return tib_errno;
    }

  char *s = tib_Expression_as_str (translated);
  tib_Expression_decref (translated);
  if (NULL == s)
    {
      fprintf (stderr, "tibdecode: Error %d occurred while processing\n",
	       tib_errno);
      return tib_errno;
    }

  fprintf (out, "%s\n", s);
  free (s);

  return 0;
}
