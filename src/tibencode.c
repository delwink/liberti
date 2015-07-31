/*
 *  tibencode - Compile a TI BASIC program
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
#include <unistd.h>
#include <getopt.h>

#include "tibtranscode.h"
#include "tiberr.h"

#define VERSION_INFO "tibencode (Delwink LiberTI) 1.0.0\n\
Copyright (C) 2015 Delwink, LLC\n\
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>.\n\
This is libre software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by David McMackins II."

int
main (int argc, char *argv[])
{
  FILE *out = stdout;
  char *inpath = NULL;
  size_t written;

  struct option longopts[] =
    {
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "v", longopts, &longindex)) != -1)
	{
	  switch (c)
	    {
	    case 'v':
	      puts (VERSION_INFO);
	      return 0;

	    case '?':
	      return 1;
	    }
	}
    }

  if (optind < argc)
    inpath = argv[optind];

  tib_Expression *translated = NULL; /* TODO: Convert expression from string */

  tib_errno = tib_fwrite (inpath, translated, &written);

  if (tib_errno)
    {
      fprintf (stderr, "tibencode: Error %d occurred while processing. "
	       "Write %lu characters.\n",
	       tib_errno, written);
      return tib_errno;
    }

  /* TODO: print translated expression */
  fprintf (out, "You get nothing. You lose. Good day, sir.");

  return 0;
}
