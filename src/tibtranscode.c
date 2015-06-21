/*
 *  libtib - Read, write, and evaluate TI BASIC programs
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
#include <errno.h>
#include <limits.h>

#include "tibtranscode.h"
#include "tibchar.h"
#include "tiberr.h"

static int
need_next (int c, int *err, FILE *program, size_t *parsed)
{
  int next;

  ++(*parsed);

  if ((next = fgetc (program)) == EOF)
    {
      *err = TIB_EBADCHAR;
      return EOF;
    }

  switch (c)
    {
    case -69:
      if (10 == next)
	return TIB_CHAR_RANDINT;

      *err = TIB_EBADCHAR;
      return EOF;

    case 92:
      if (next >= 0 && next <= 8)
	return next + TIB_CHAR_MATA;

      *err = TIB_EBADCHAR;
      return EOF;

    case 93:
      if (next >= 0 && next <= 8)
	return next + TIB_CHAR_L1;

      *err = TIB_EBADCHAR;
      return EOF;

    case 96:
      if (0 == next)
	return TIB_CHAR_PIC1;

      *err = TIB_EBADCHAR;
      return EOF;

    case 99:
      switch (next)
	{
	case 10:
	  return TIB_CHAR_XMIN;

	case 11:
	  return TIB_CHAR_XMAX;

	case 12:
	  return TIB_CHAR_YMIN;

	case 13:
	  return TIB_CHAR_YMAX;

	default:
	  *err = TIB_EBADCHAR;
	  return EOF;
	}

    case 126:
      if (9 == c)
	return TIB_CHAR_AXESOFF;

      *err = TIB_EBADCHAR;
      return EOF;

    default:
      *err = TIB_EBADCHAR;
      return EOF;
    }
}

static int
trans_from (int c, int *err, FILE *program, size_t *parsed)
{
  if (c > SCHAR_MAX)
    c -= 256;

  /* The original author of LibreCalc's TI emulator had several comments in
     this conversion case analysis where he was unsure of the legitimacy of
     some character conversions. Be wary of the following: 91, 109 or 14, 114,
     98, 24, 127 or -40, 95 */
  switch (c)
    {
    case -126:
      return '*';

    case -125:
      return '/';

    case -123:
      return TIB_CHAR_CLEARDRAW;

    case -109:
      return TIB_CHAR_TEXT;

    case -104:
      return TIB_CHAR_STOREPIC;

    case -103:
      return TIB_CHAR_RECALLPIC;

    case -100:
      return TIB_CHAR_LINE;

    case -85:
      return TIB_CHAR_RAND;

    case -84:
      return TIB_CHAR_PI;

    case -83:
      return TIB_CHAR_GETKEY;

    case -81:
      return '?';

    case -80:
      return TIB_CHAR_SMALL_MINUS;

    case -79:
      return TIB_CHAR_INT;

    case -75:
      return TIB_CHAR_DIM;

    case -72:
      return TIB_CHAR_NOT;

    case -69:
      return need_next (c, err, program, parsed);

    case -50:
      return TIB_CHAR_IF;

    case -49:
      return TIB_CHAR_THEN;

    case -48:
      return TIB_CHAR_ELSE;

    case -47:
      return TIB_CHAR_WHILE;

    case -46:
      return TIB_CHAR_REPEAT;

    case -45:
      return TIB_CHAR_FOR;

    case -44:
      return TIB_CHAR_END;

    case -43:
      return TIB_CHAR_RETURN;

    case -42:
      return TIB_CHAR_LABEL;

    case -41:
      return TIB_CHAR_GOTO;

    case -40:
      return TIB_CHAR_PAUSE;

    case -39:
      return TIB_CHAR_STOP;

    case -36:
      return TIB_CHAR_INPUT;

    case -34:
      return TIB_CHAR_DISP;

    case -32:
      return TIB_CHAR_OUTPUT;

    case -31:
      return TIB_CHAR_CLEARHOME;

    case -30:
      return TIB_CHAR_FILL;

    case -26:
      return TIB_CHAR_MENU;

    case -21:
      return TIB_CHAR_LUSER;

    case -16:
      return '^';

    case -6:
      return TIB_CHAR_CLRLIST;

    case 4:
      return TIB_CHAR_STO;

    case 6:
      return '[';

    case 7:
      return ']';

    case 8:
      return '{';

    case 9:
      return '}';

    case 11:
      return TIB_CHAR_DEGREE;

    case 14:
      return 'T';

    case 16:
      return '(';

    case 17:
      return ')';

    case 18:
      return TIB_CHAR_ROUND;

    case 19:
      return TIB_CHAR_PIXEL_TEST;

    case 22:
      return '|';

    /* 24 was simply skipped in the original emulator; could cause problems */

    case 41:
      return ' ';

    case 42:
      return '"';

    case 43:
      return ',';

    case 45:
      return '!';

    case 58:
      return '.';

    case 59:
      return TIB_CHAR_EPOW10;

    case 60:
      return TIB_CHAR_OR;

    case 62:
    case 63:
      return '\n';

    case 64:
      return TIB_CHAR_AND;

    case 91:
      return TIB_CHAR_THETA;

    case 92:
    case 93:
      return need_next (c, err, program, parsed);

    /* 95 was not evaluated, but it printed "Pause  " to stdout */

    case 96:
      return need_next (c, err, program, parsed);

    case 98:
      return 'c';

    case 99:
      return need_next (c, err, program, parsed);

    case 106:
      return '=';

    case 107:
      return '<';

    case 108:
      return '>';

    case 109:
      return TIB_CHAR_LESSEQUAL;

    case 110:
      return TIB_CHAR_GREATEREQUAL;

    case 111:
      return TIB_CHAR_DIFFERENT;

    case 112:
      return '+';

    case 113:
      return '-';

    case 114:
      return TIB_CHAR_ANS;

    case 126:
      return need_next (c, err, program, parsed);

    case 127:
      return '[';

    default:
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))
	{
	  return c;
	}
      else if (fgetc (program) != EOF || fgetc (program) != EOF)
	{
	  *err = 0;
	  *parsed += 2;
	  return EOF;
	}

      *err = TIB_EBADCHAR;
      return EOF;
    }
}

tib_Expression *
tib_fread (const char *path, size_t *parsed)
{
  FILE *program = stdin;
  tib_Expression *out;
  int c;

  if (path)
    {
      program = fopen (path, "rb");
      if (NULL == program)
	{
	  tib_errno = errno;
	  return NULL;
	}
    }

  out = tib_new_Expression ();
  if (NULL == out)
    {
      if (path)
	fclose (program);
      return NULL;
    }

  *parsed = 0;
  while ((c = fgetc (program)) != EOF && *parsed < 71)
    ++(*parsed);

  if (*parsed != 71)
    {
      if (path)
	fclose (program);
      tib_Expression_decref (out);
      tib_errno = TIB_EBADFILE;
      return NULL;
    }

  while ((c = fgetc (program)) != EOF)
    {
      ++(*parsed);

      int trans = trans_from ((char) c, &tib_errno, program, parsed);
      if (tib_errno)
	break;

      tib_errno = tib_Expression_push (out, trans);
      if (tib_errno)
	break;
    }

  if (path)
    fclose (program);

  if (tib_errno)
    {
      tib_Expression_decref (out);
      return NULL;
    }

  return out;
}

int
tib_fwrite (const char *path, const tib_Expression *program, size_t *written)
{
  FILE *out = stdout;
  int rc;
  size_t i;

  if (path)
    {
      out = fopen (path, "wb");
      if (NULL == out)
	return errno;
    }

  for (*written = 0; *written <= 71; ++(*written))
    {
      rc = fputc (0, out);
      if (rc)
	break;
    }

  if (rc)
    {
      if (path)
	fclose (out);
      return TIB_EWRITE;
    }

  tib_foreachexpr (program, i)
    {
      int c = tib_Expression_ref (program, i);
      switch (c)
	{

	default:
	  if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))
	    rc = fputc (c, out);
	  else
	    rc = EOF;
	  break;
	}

      if (EOF == rc)
	break;

      ++(*written);
    }

  if (path)
    fclose (out);

  return EOF == rc ? TIB_EWRITE : 0;
}
