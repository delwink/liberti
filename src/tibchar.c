/*
 *  libtib - Read, write, and evaluate TI BASIC programs
 *  Copyright (C) 2015-2016 Delwink, LLC
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
#include <stdlib.h>
#include <string.h>

#include "tibchar.h"
#include "tiberr.h"

static PrefixTree *keywords = NULL;

const char *
tib_special_char_text (int c)
{
  switch (c)
    {
    case TIB_CHAR_AND:
      return " And ";

    case TIB_CHAR_ANS:
      return "Ans";

    case TIB_CHAR_AXESOFF:
      return "AxesOff";

    case TIB_CHAR_CLEARDRAW:
      return "ClrDraw";

    case TIB_CHAR_CLEARHOME:
      return "ClrHome";

    case TIB_CHAR_CLRLIST:
      return "ClrList";

    case TIB_CHAR_DELVAR:
      return "DelVar";

    case TIB_CHAR_DIFFERENT:
      return "~";

    case TIB_CHAR_DIM:
      return "Dim(";

    case TIB_CHAR_DISP:
      return "Disp(";

    case TIB_CHAR_ELSE:
      return "Else";

    case TIB_CHAR_END:
      return "End";

    case TIB_CHAR_EPOW10:
      return "*10^";

    case TIB_CHAR_FILL:
      return "Fill(";

    case TIB_CHAR_FOR:
      return "For(";

    case TIB_CHAR_GETKEY:
      return "GetKey";

    case TIB_CHAR_GOTO:
      return "Goto ";

    case TIB_CHAR_IF:
      return "If ";

    case TIB_CHAR_INPUT:
      return "Input ";

    case TIB_CHAR_INT:
      return "int(";

    case TIB_CHAR_L1:
      return "L\\1";

    case TIB_CHAR_L2:
      return "L\\2";

    case TIB_CHAR_L3:
      return "L\\3";

    case TIB_CHAR_L4:
      return "L\\4";

    case TIB_CHAR_L5:
      return "L\\5";

    case TIB_CHAR_L6:
      return "L\\6";

    case TIB_CHAR_L7:
      return "L\\7";

    case TIB_CHAR_L8:
      return "L\\8";

    case TIB_CHAR_L9:
      return "L\\9";

    case TIB_CHAR_LABEL:
      return "Lbl ";

    case TIB_CHAR_LINE:
      return "Line(";

    case TIB_CHAR_MATA:
      return "[[A]]";

    case TIB_CHAR_MATB:
      return "[[B]]";

    case TIB_CHAR_MATC:
      return "[[C]]";

    case TIB_CHAR_MATD:
      return "[[D]]";

    case TIB_CHAR_MATE:
      return "[[E]]";

    case TIB_CHAR_MATF:
      return "[[F]]";

    case TIB_CHAR_MATG:
      return "[[G]]";

    case TIB_CHAR_MATH:
      return "[[H]]";

    case TIB_CHAR_MATI:
      return "[[I]]";

    case TIB_CHAR_MENU:
      return "Menu(";

    case TIB_CHAR_NOT:
      return "Not(";

    case TIB_CHAR_OR:
      return " Or ";

    case TIB_CHAR_OUTPUT:
      return "Output(";

    case TIB_CHAR_PAUSE:
      return "Pause ";

    case TIB_CHAR_PIC1:
      return "Pic1";

    case TIB_CHAR_PIXEL_TEST:
      return "pxl-Test(";

    case TIB_CHAR_RAND:
      return "RAND";

    case TIB_CHAR_RANDINT:
      return "RandInt(";

    case TIB_CHAR_RECALLPIC:
      return "RecallPic ";

    case TIB_CHAR_REPEAT:
      return "Repeat ";

    case TIB_CHAR_RETURN:
      return "Return ";

    case TIB_CHAR_ROUND:
      return "Round(";

    case TIB_CHAR_SIN:
      return "sin(";

    case TIB_CHAR_STO:
      return "$";

    case TIB_CHAR_STOP:
      return "Stop ";

    case TIB_CHAR_STOREPIC:
      return "StorePic ";

    case TIB_CHAR_TEXT:
      return "Text(";

    case TIB_CHAR_THEN:
      return "Then";

    case TIB_CHAR_THETA:
      return "Theta";

    case TIB_CHAR_WHILE:
      return "While ";

    case TIB_CHAR_XMIN:
      return "Xmin";

    case TIB_CHAR_XMAX:
      return "Xmax";

    case TIB_CHAR_XSCL:
      return "Xscl";

    case TIB_CHAR_YMIN:
      return "Ymin";

    case TIB_CHAR_YMAX:
      return "Ymax";

    case TIB_CHAR_YSCL:
      return "Yscl";

    default:
      return NULL;
    }
}

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
tokenize (struct tib_expr *expr, char *beg)
{
  int rc;
  char *orig = beg;
  size_t len = strlen (beg);

  rc = tib_expr_init (expr);
  if (rc)
    return rc;

  while (beg < orig + len)
    {
      char temp;
      char *end = strchr (beg, '(');

      if (end)
	{
	  temp = *(++end);
	  *end = '\0';

	  const PrefixTree *t = pt_search (keywords, beg);
	  *end = temp;

	  if (t)
	    {
	      rc = tib_expr_push (expr, pt_data (t));
	      if (rc)
		goto fail;

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
	      rc = tib_expr_push (expr, pt_data (t));
	      if (rc)
		goto fail;

	      beg = end;
	      found = true;
	    }

	  *end = temp;
	  if (found)
	    break;
	}

      if (!found)
	{
	  rc = tib_expr_push (expr, *beg);
	  if (rc)
	    goto fail;

	  ++beg;
	}
    }

  return rc;

 fail:
  tib_expr_free_data (expr);
  return rc;
}

int
tib_encode_str (struct tib_expr *expr, const char *s)
{
  int rc = 0;
  char *buf, *beg;
  size_t line_len = strlen (s);

  buf = malloc ((line_len + 1) * sizeof (char));
  if (!buf)
    return TIB_EALLOC;

  strcpy (buf, s);
  beg = buf;
  expr->len = 0;

  while (beg < buf + line_len)
    {
      char *end = strchr (beg, '"');
      if (end)
	*end = '\0';

      struct tib_expr part;
      rc = tokenize (&part, beg);
      if (rc)
	break;

      rc = tib_exprcat (expr, &part);
      tib_expr_free_data (&part);
      if (rc)
	break;

      if (!end)
	break;

      beg = end + 1;
      end = strchr (beg, '"');

      if (!end)
	end = buf + line_len;

      rc = tib_expr_push (expr, '"');
      if (rc)
	break;

      for (; beg < end; ++beg)
	{
	  rc = tib_expr_push (expr, *beg);
	  if (rc)
	    goto end;
	}

      if (*end)
	{
	  rc = tib_expr_push (expr, *end);
	  if (rc)
	    break;
	}

      beg = end + 1;
    }

 end:
  if (rc)
    tib_expr_free_data (expr);

  free (buf);
  return rc;
}

int
tib_keyword_init ()
{
  int rc;

  if (keywords)
    pt_free (keywords);

  keywords = pt_new ();
  if (NULL == keywords)
    return TIB_EALLOC;

  rc = load_range (TIB_CHAR_AND, TIB_CHAR_YSCL);
  if (rc)
    tib_keyword_free ();

  return rc;
}

void
tib_keyword_free ()
{
  if (keywords)
    {
      pt_free (keywords);
      keywords = NULL;
    }
}
