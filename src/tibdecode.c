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

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tiberr.h"
#include "tibtranscode.h"

#define USAGE_INFO "USAGE: tibdecode [options]\n\n\
tibdecode reads a TI-82 or TI-83 program from stdin and prints to stdout.\n\n\
OPTIONS:\n\
\t-d, --debug\tShows the decimal value of unknown characters in {}\n\
\t-h, --help\tPrints this help message and exits\n\
\t-v, --version\tPrints version info and exits\n"

#define VERSION_INFO "tibdecode (Delwink LiberTI) 1.0.0\n\
Copyright (C) 2015 Delwink, LLC\n\
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>.\n\
This is libre software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by David McMackins II."

#if _BSD_SOURCE || _SVID_SOURCE || _XOPEN_SOURCE
# include <ctype.h>
#else
static int
isascii (int c)
{
  return c >= 0 && c < 128;
}
#endif

int
main (int argc, char *argv[])
{
  bool debug = false;
  unsigned long parsed;
  size_t i;

  struct option longopts[] =
    {
      {"debug",   no_argument, 0, 'd'},
      {"help",    no_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  if (argc > 1)
    {
      int c;
      int longindex;
      while ((c = getopt_long (argc, argv, "dhv", longopts, &longindex)) != -1)
	{
	  switch (c)
	    {
	    case 'd':
	      debug = true;
	      break;

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

  tib_Expression *translated = tib_fread (stdin, &parsed);
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

  size_t len = strlen (s);
  for (i = 0; i < len; ++i)
    {
      if (debug && !isascii (s[i]))
	printf ("`%d`", s[i]);
      else
	putchar (s[i]);
    }

  free (s);

  return 0;
}
