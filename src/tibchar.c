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

#include <stdlib.h>

#include "tibchar.h"

const char *
tib_special_char_text (int c)
{
  switch (c)
    {
    case TIB_CHAR_SIN:
      return "sin(";

    case TIB_CHAR_IF:
      return "If ";

    case TIB_CHAR_END:
      return "End";

    case TIB_CHAR_THEN:
      return "Then";

    case TIB_CHAR_ELSE:
      return "Else";

    case TIB_CHAR_WHILE:
      return "While ";

    case TIB_CHAR_OUTPUT:
      return "Output(";

    case TIB_CHAR_CLEARHOME:
      return "ClrHome";

    case TIB_CHAR_RANDINT:
      return "RandInt(";

    case TIB_CHAR_AND:
      return " And ";

    case TIB_CHAR_OR:
      return " Or ";

    case TIB_CHAR_GETKEY:
      return "GetKey";

    case TIB_CHAR_FOR:
      return "For(";

    case TIB_CHAR_LABEL:
      return "Lbl ";

    case TIB_CHAR_GOTO:
      return "Goto ";

    case TIB_CHAR_NOT:
      return "Not(";

    case TIB_CHAR_PAUSE:
      return "Pause ";

    case TIB_CHAR_REPEAT:
      return "Repeat ";

    case TIB_CHAR_ANS:
      return "Ans";

    case TIB_CHAR_RAND:
      return "RAND";

    case TIB_CHAR_ROUND:
      return "Round(";

    case TIB_CHAR_INT:
      return "int(";

    case TIB_CHAR_DELVAR:
      return "DelVar";

    case TIB_CHAR_DISP:
      return "Disp(";

    case TIB_CHAR_RETURN:
      return "Return ";

    case TIB_CHAR_MENU:
      return "Menu(";

    case TIB_CHAR_DIM:
      return "Dim(";

    case TIB_CHAR_FILL:
      return "Fill(";

    case TIB_CHAR_CLRLIST:
      return "ClrList";

    case TIB_CHAR_INPUT:
      return "Input ";

    case TIB_CHAR_STOP:
      return "Stop ";

    case TIB_CHAR_L1:
      return "L&";

    case TIB_CHAR_L2:
      return "L~";

    case TIB_CHAR_L3:
      return "L#";

    case TIB_CHAR_L4:
      return "L\x8c";

    case TIB_CHAR_L5:
      return "L\x8b";

    case TIB_CHAR_L6:
      return "L\x8a";

    case TIB_CHAR_L7:
      return "L\x89";

    case TIB_CHAR_L8:
      return "L\x88";

    case TIB_CHAR_L9:
      return "L\x87";

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

    case TIB_CHAR_CLEARDRAW:
      return "ClrDraw";

    case TIB_CHAR_AXESOFF:
      return "AxesOff";

    case TIB_CHAR_YMIN:
      return "YMin";

    case TIB_CHAR_YMAX:
      return "YMax";

    case TIB_CHAR_XMIN:
      return "XMin";

    case TIB_CHAR_XMAX:
      return "XMax";

    case TIB_CHAR_LINE:
      return "Line(";

    case TIB_CHAR_STOREPIC:
      return "StorePic ";

    case TIB_CHAR_RECALLPIC:
      return "RecallPic ";

    case TIB_CHAR_PIC1:
      return "Pic1";

    case TIB_CHAR_TEXT:
      return "Text(";

    case TIB_CHAR_PIXEL_TEST:
      return "pxl-Test(";

    default:
      return NULL;
    }

  return NULL; /* should not be able to get here */
}
