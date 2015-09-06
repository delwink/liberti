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

#include <pfxtree.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "tibchar.h"
#include "tibtranscode.h"
#include "tiberr.h"

#define VERSION_INFO "tibencode (Delwink LiberTI) 1.0.0\n\
Copyright (C) 2015 Delwink, LLC\n\
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>.\n\
This is libre software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by David McMackins II."

static PrefixTree *keywords = NULL;

static int
load_range (int beg, int end)
{
  int rc = 0, i;
  for (i = beg; i <= end; ++i)
    {
      const char *trans = tib_special_char_text (i);
      if (NULL == trans)
	continue;

      rc = pt_add (keywords, trans, i);
      if (rc)
	return rc;
    }

  return rc;
}

static int
load_keywords ()
{
  int rc;

  if (keywords)
    pt_free (keywords);

  keywords = pt_new ();
  if (NULL == keywords)
    return TIB_EALLOC;

  rc = load_range (TIB_CHAR_LUSER, TIB_CHAR_GREATEREQUAL);
  if (rc)
    goto fail;

  rc = load_range (TIB_CHAR_SIN, TIB_CHAR_PIXEL_TEST);
  if (rc)
    goto fail;

  return rc;

 fail:
  if (keywords)
    {
      pt_free (keywords);
      keywords = NULL;
    }

  return rc;
}

static tib_Expression *
tokenize (char *beg)
{
  char temp;
  char *orig = beg;
  size_t len = strlen (beg);

  tib_Expression *part = tib_new_Expression ();
  if (NULL == part)
    return NULL;

  while (beg < orig + len)
    {
      char *end = strchr (beg, '(');
      if (end)
	{
	  ++end;
	  temp = *end;
	  *end = '\0';
	  const PrefixTree *t = pt_search (keywords, beg);
	  *end = temp;
	  if (t)
	    {
	      tib_errno = tib_Expression_push (part, pt_data (t));
	      if (tib_errno)
		break;

	      beg = end;
	      continue;
	    }
	}

      bool found = false;
      for (end = beg + 1; end <= orig + len; ++end)
	{
	  temp = *end;
	  *end = '\0';

	  const PrefixTree *t = pt_search (keywords, beg);
	  if (t)
	    {
	      tib_errno = tib_Expression_push (part, pt_data (t));
	      if (tib_errno)
		break;

	      beg = end;
	      found = true;
	    }

	  *end = temp;
	  if (found)
	    break;
	}

      if (tib_errno)
	break;

      if (found)
	continue;

      tib_errno = tib_Expression_push (part, *beg);
      if (tib_errno)
	break;

      ++beg;
    }

  if (tib_errno)
    {
      tib_Expression_decref (part);
      part = NULL;
    }

  return part;
}

int
main (int argc, char *argv[])
{
  unsigned long written;

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

  tib_errno = load_keywords ();
  if (tib_errno)
    {
      fputs ("tibencode: Error allocating space for keyword tree.", stderr);
      return tib_errno;
    }

  tib_Expression *translated = tib_new_Expression ();
  if (NULL == translated)
    {
      pt_free (keywords);
      fputs ("tibencode: Error creating expression buffer.", stderr);
      return TIB_EALLOC;
    }

  size_t max_line_len = 128;
  char *buf = malloc (max_line_len * sizeof (char));
  if (NULL == buf)
    {
      tib_Expression_decref (translated);
      pt_free (keywords);
      fputs ("tibencode: Error allocating line buffer.", stderr);
      return TIB_EALLOC;
    }

  int c = '\0';
  size_t line_len = 0;
  while (c != EOF)
    {
      char *beg = buf, *end = buf;

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

      while (beg < buf + line_len)
	{
	  end = strchr (beg, '"');
	  if (end)
	    *end = '\0';

	  tib_Expression *part = tokenize (beg);
	  if (NULL == part && tib_errno)
	    break;

	  tib_errno = tib_Expression_cat (translated, part);
	  tib_Expression_decref (part);
	  if (tib_errno)
	    break;

	  if (end)
	    {
	      beg = end + 1;
	      end = strchr (beg, '"');

	      if (!end)
		end = buf + line_len;

	      tib_errno = tib_Expression_push (translated, '"');
	      if (tib_errno)
		break;

	      for (; beg < end; ++beg)
		{
		  tib_errno = tib_Expression_push (translated, *beg);
		  if (tib_errno)
		    break;
		}
	      if (tib_errno)
		break;

	      if (*end)
		{
		  tib_errno = tib_Expression_push (translated, *end);
		  if (tib_errno)
		    break;
		}

	      beg = end + 1;
	    }
	  else
	    {
	      break;
	    }
	}

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
  pt_free (keywords);

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
