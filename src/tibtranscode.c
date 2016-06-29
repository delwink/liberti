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

#include <stdio.h>
#include <limits.h>

#include "tibtranscode.h"
#include "tibchar.h"
#include "tiberr.h"

static int
need_next (int c, int *err, FILE *program, unsigned long *parsed)
{
  int next;

  if ((next = fgetc (program)) == EOF)
    {
      *err = TIB_EBADCHAR;
      return EOF;
    }

  ++(*parsed);

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
trans_from (int c, int *err, FILE *program, unsigned long *parsed)
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
    case 24:
      return 0;

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

int
tib_fread (struct tib_expr *out, FILE *program, unsigned long *parsed)
{
  int rc, c;

  rc = tib_expr_init (out);
  if (rc)
    return rc;

  *parsed = 0;
  while ((c = fgetc (program)) != EOF && *parsed < 71)
    ++(*parsed);

  if (*parsed != 71)
    {
      rc = TIB_EBADFILE;
      goto end;
    }

  while ((c = fgetc (program)) != EOF)
    {
      ++(*parsed);

      int trans = trans_from ((char) c, &rc, program, parsed);
      if (EOF == trans)
        break;

      if (trans)
        {
          rc = tib_expr_push (out, trans);
          if (rc)
            break;
        }
    }

 end:
  if (rc)
    tib_expr_destroy (out);

  return rc;
}

int
tib_fwrite (FILE *out, const struct tib_expr *program, unsigned long *written)
{
  int rc;
  unsigned int i;

  for (*written = 0; *written <= 71; ++(*written))
    {
      rc = fputc (0, out);
      if (rc)
        return TIB_EWRITE;
    }

  tib_expr_foreach (program, i)
    {
      int c = program->data[i];
      /* The original conversion table had a "TODO transpose" comment. May be
         an incomplete table. */
      switch (c)
        {
        case '\n':
          rc = fputc (62, out);
          break;

        case ' ':
          rc = fputc (41, out);
          break;

        case '!':
          rc = fputc (45, out);
          break;

        case '"':
          rc = fputc (42, out);
          break;

        case '(':
          rc = fputc (16, out);
          break;

        case ')':
          rc = fputc (17, out);
          break;

        case '*':
          rc = fputc (-126, out);
          break;

        case '+':
          rc = fputc (112, out);
          break;

        case ',':
          rc = fputc (43, out);
          break;

        case '-':
          rc = fputc (113, out);
          break;

        case '.':
          rc = fputc (58, out);
          break;

        case '/':
          rc = fputc (-125, out);
          break;

        case '<':
          rc = fputc (107, out);
          break;

        case '=':
          rc = fputc (106, out);
          break;

        case '>':
          rc = fputc (108, out);
          break;

        case '?':
          rc = fputc (-81, out);
          break;

        case '[':
          rc = fputc (6, out);
          break;

        case ']':
          rc = fputc (7, out);
          break;

        case '^':
          rc = fputc (-16, out);
          break;

        case '{':
          rc = fputc (8, out);
          break;

        case '|':
          rc = fputc (22, out);
          break;

        case '}':
          rc = fputc (9, out);
          break;

        case TIB_CHAR_AND:
          rc = fputc (64, out);
          break;

        case TIB_CHAR_ANS:
          rc = fputc (114, out);
          break;

        case TIB_CHAR_AXESOFF:
          rc = fputc (126, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (9, out);
          break;

        case TIB_CHAR_CLEARHOME:
          rc = fputc (-31, out);
          break;

        case TIB_CHAR_CLEARDRAW:
          rc = fputc (-123, out);
          break;

        case TIB_CHAR_CLRLIST:
          rc = fputc (-6, out);
          break;

        case TIB_CHAR_DEGREE:
          rc = fputc (11, out);
          break;

        case TIB_CHAR_DIFFERENT:
          rc = fputc (111, out);
          break;

        case TIB_CHAR_DIM:
          rc = fputc (-75, out);
          break;

        case TIB_CHAR_DISP:
          rc = fputc (-34, out);
          break;

        case TIB_CHAR_ELSE:
          rc = fputc (-48, out);
          break;

        case TIB_CHAR_END:
          rc = fputc (-44, out);
          break;

        case TIB_CHAR_EPOW10:
          rc = fputc (59, out);
          break;

        case TIB_CHAR_FILL:
          rc = fputc (-30, out);
          break;

        case TIB_CHAR_FOR:
          rc = fputc (-45, out);
          break;

        case TIB_CHAR_GETKEY:
          rc = fputc (-83, out);
          break;

        case TIB_CHAR_GOTO:
          rc = fputc (-41, out);
          break;

        case TIB_CHAR_GREATEREQUAL:
          rc = fputc (110, out);
          break;

        case TIB_CHAR_IF:
          rc = fputc (-50, out);
          break;

        case TIB_CHAR_INPUT:
          rc = fputc (-36, out);
          break;

        case TIB_CHAR_INT:
          rc = fputc (-79, out);
          break;

        case TIB_CHAR_L1:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (0, out);
          break;

        case TIB_CHAR_L2:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (1, out);
          break;

        case TIB_CHAR_L3:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (2, out);
          break;

        case TIB_CHAR_L4:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (3, out);
          break;

        case TIB_CHAR_L5:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (4, out);
          break;

        case TIB_CHAR_L6:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (5, out);
          break;

        case TIB_CHAR_L7:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (6, out);
          break;

        case TIB_CHAR_L8:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (7, out);
          break;

        case TIB_CHAR_L9:
          rc = fputc (93, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (8, out);
          break;

        case TIB_CHAR_LABEL:
          rc = fputc (-42, out);
          break;

        case TIB_CHAR_LESSEQUAL:
          rc = fputc (109, out);
          break;

        case TIB_CHAR_LINE:
          rc = fputc (-100, out);
          break;

        case TIB_CHAR_LUSER:
          rc = fputc (-21, out);
          break;

        case TIB_CHAR_MATA:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (0, out);
          break;

        case TIB_CHAR_MATB:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (1, out);
          break;

        case TIB_CHAR_MATC:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (2, out);
          break;

        case TIB_CHAR_MATD:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (3, out);
          break;

        case TIB_CHAR_MATE:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (4, out);
          break;

        case TIB_CHAR_MATF:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (5, out);
          break;

        case TIB_CHAR_MATG:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (6, out);
          break;

        case TIB_CHAR_MATH:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (7, out);
          break;

        case TIB_CHAR_MATI:
          rc = fputc (92, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (8, out);
          break;

        case TIB_CHAR_MENU:
          rc = fputc (-26, out);
          break;

        case TIB_CHAR_NOT:
          rc = fputc (-72, out);
          break;

        case TIB_CHAR_OUTPUT:
          rc = fputc (-32, out);
          break;

        case TIB_CHAR_OR:
          rc = fputc (60, out);
          break;

        case TIB_CHAR_PAUSE:
          rc = fputc (-40, out);
          break;

        case TIB_CHAR_PI:
          rc = fputc (-84, out);
          break;

        case TIB_CHAR_PIC1:
          rc = fputc (96, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (0, out);
          break;

        case TIB_CHAR_PIXEL_TEST:
          rc = fputc (19, out);
          break;

        case TIB_CHAR_RAND:
          rc = fputc (-85, out);
          break;

        case TIB_CHAR_RANDINT:
          rc = fputc (-69, out);
          if (EOF == rc)
            break;
          rc = fputc (10, out);
          break;

        case TIB_CHAR_RECALLPIC:
          rc = fputc (-103, out);
          break;

        case TIB_CHAR_REPEAT:
          rc = fputc (-46, out);
          break;

        case TIB_CHAR_RETURN:
          rc = fputc (-43, out);
          break;

        case TIB_CHAR_ROUND:
          rc = fputc (18, out);
          break;

        case TIB_CHAR_SMALL_MINUS:
          rc = fputc (-80, out);
          break;

        case TIB_CHAR_STO:
          rc = fputc (4, out);
          break;

        case TIB_CHAR_STOP:
          rc = fputc (-39, out);
          break;

        case TIB_CHAR_STOREPIC:
          rc = fputc (-104, out);
          break;

        case TIB_CHAR_TEXT:
          rc = fputc (-109, out);
          break;

        case TIB_CHAR_THEN:
          rc = fputc (-49, out);
          break;

        case TIB_CHAR_THETA:
          rc = fputc (91, out);
          break;

        case TIB_CHAR_WHILE:
          rc = fputc (-47, out);
          break;

        case TIB_CHAR_XMAX:
          rc = fputc (99, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (11, out);
          break;

        case TIB_CHAR_XMIN:
          rc = fputc (99, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (10, out);
          break;

        case TIB_CHAR_YMAX:
          rc = fputc (99, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (13, out);
          break;

        case TIB_CHAR_YMIN:
          rc = fputc (99, out);
          if (EOF == rc)
            break;
          ++(*written);
          rc = fputc (12, out);
          break;

        default:
          if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))
            rc = fputc (c, out);
          else
            rc = EOF;
          break;
        }

      if (EOF == rc)
        return TIB_EWRITE;

      ++(*written);
    }

  return 0;
}
