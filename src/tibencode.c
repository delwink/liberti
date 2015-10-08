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
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "tibchar.h"
#include "tibtranscode.h"
#include "tiberr.h"

#define USAGE_INFO "USAGE: tibencode [options]\n\n\
tibencode reads a TI-BASIC program from stdin and prints to stdout.\n\n\
OPTIONS:\n\
\t-h, --help\tPrints this help message and exits\n\
\t-v, --version\tPrints version info and exits\n"

#define VERSION_INFO "tibencode (Delwink LiberTI) 1.0.0\n\
Copyright (C) 2015 Delwink, LLC\n\
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>.\n\
This is libre software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by David McMackins II."

int
main (int argc, char *argv[])
{
  unsigned long written;

  struct option longopts[] =
    {
      {"help",    no_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "hv", longopts, &longindex)) != -1)
	{
	  switch (c)
	    {
	    case 'h':
	      puts (USAGE_INFO);
	      return 0;

	    case 'v':
	      puts (VERSION_INFO);
	      return 0;

	    case '?':
	      return 1;
	    }
	}
    }

  tib_errno = tib_keyword_init ();
  if (tib_errno)
    {
      fputs ("tibencode: Error allocating space for keyword tree.\n", stderr);
      return tib_errno;
    }

  tib_Expression *translated = tib_new_Expression ();
  if (NULL == translated)
    {
      fputs ("tibencode: Error creating expression buffer.\n", stderr);
      return TIB_EALLOC;
    }

  size_t max_line_len = 128;
  char *buf = malloc (max_line_len * sizeof (char));
  if (NULL == buf)
    {
      tib_Expression_decref (translated);
      fputs ("tibencode: Error allocating line buffer.\n", stderr);
      return TIB_EALLOC;
    }

  int c = '\0';
  size_t line_len = 0;
  while (c != EOF)
    {
      for (; line_len < max_line_len - 1; ++line_len)
	{
	  c = getchar ();
	  if ('\n' == c || EOF == c)
	    break;

	  buf[line_len] = c;
	}

      if (max_line_len - 1 == line_len)
	{
	  max_line_len *= 2;
	  buf = realloc (buf, max_line_len * sizeof (char));
	  continue;
	}

      buf[line_len] = '\0';

      tib_Expression *line = tib_encode_str (buf);
      if (NULL == line)
	break;

      tib_errno = tib_Expression_cat (translated, line);
      tib_Expression_decref (line);
      if (tib_errno)
	break;

      if (c != EOF)
	{
	  tib_errno = tib_Expression_push (translated, '\n');
	  if (tib_errno)
	    break;
	}

      line_len = 0;
    }

  free (buf);
  tib_keyword_free ();

  if (tib_errno)
    {
      tib_Expression_decref (translated);
      fprintf (stderr, "tibencode: Error %d occurred while assembling.\n",
	       tib_errno);
      return tib_errno;
    }

  tib_errno = tib_fwrite (stdout, translated, &written);
  tib_Expression_decref (translated);
  if (tib_errno)
    {
      fprintf (stderr, "tibencode: Error %d occurred while processing. "
	       "Wrote %lu characters.\n",
	       tib_errno, written);
      return tib_errno;
    }

  return 0;
}
