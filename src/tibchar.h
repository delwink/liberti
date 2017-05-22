/*
 *  libtib - Read, write, and evaluate TI BASIC programs
 *  Copyright (C) 2015-2017 Delwink, LLC
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

#ifndef DELWINK_TIB_TIBCHAR_H
#define DELWINK_TIB_TIBCHAR_H

#include "tibexpr.h"

enum tib_special_char
{
	/* printing characters */
	TIB_CHAR_DEGREE = 128,
	TIB_CHAR_EPOW10,
	TIB_CHAR_LESSEQUAL,
	TIB_CHAR_GREATEREQUAL,
	TIB_CHAR_L1,
	TIB_CHAR_L2,
	TIB_CHAR_L3,
	TIB_CHAR_L4,
	TIB_CHAR_L5,
	TIB_CHAR_L6,
	TIB_CHAR_L7,
	TIB_CHAR_L8,
	TIB_CHAR_L9,
	TIB_CHAR_PI,
	TIB_CHAR_SMALL1,
	TIB_CHAR_SMALL2,
	TIB_CHAR_SMALL3,
	TIB_CHAR_SMALL4,
	TIB_CHAR_SMALL5,
	TIB_CHAR_SMALL6,
	TIB_CHAR_SMALL7,
	TIB_CHAR_SMALL8,
	TIB_CHAR_SMALL9,
	TIB_CHAR_SMALL_MINUS,
	TIB_CHAR_STO,
	TIB_CHAR_THETA,

	/* "characters" expanded to strings */
	TIB_CHAR_AND,
	TIB_CHAR_ANS,
	TIB_CHAR_AXESOFF,
	TIB_CHAR_AXESON,
	TIB_CHAR_CLEARDRAW,
	TIB_CHAR_CLEARHOME,
	TIB_CHAR_CLRLIST,
	TIB_CHAR_COS,
	TIB_CHAR_DELVAR,
	TIB_CHAR_DIFFERENT,
	TIB_CHAR_DIM,
	TIB_CHAR_DISP,
	TIB_CHAR_ELSE,
	TIB_CHAR_END,
	TIB_CHAR_FILL,
	TIB_CHAR_FOR,
	TIB_CHAR_GETKEY,
	TIB_CHAR_GOTO,
	TIB_CHAR_IF,
	TIB_CHAR_INPUT,
	TIB_CHAR_INT,
	TIB_CHAR_LABEL,
	TIB_CHAR_LINE,
	TIB_CHAR_LUSER,
	TIB_CHAR_MATA,
	TIB_CHAR_MATB,
	TIB_CHAR_MATC,
	TIB_CHAR_MATD,
	TIB_CHAR_MATE,
	TIB_CHAR_MATF,
	TIB_CHAR_MATG,
	TIB_CHAR_MATH,
	TIB_CHAR_MATI,
	TIB_CHAR_MENU,
	TIB_CHAR_NOT,
	TIB_CHAR_OR,
	TIB_CHAR_OUTPUT,
	TIB_CHAR_PAUSE,
	TIB_CHAR_PIC1,
	TIB_CHAR_PIXEL_TEST,
	TIB_CHAR_RAND,
	TIB_CHAR_RANDINT,
	TIB_CHAR_RECALLPIC,
	TIB_CHAR_REPEAT,
	TIB_CHAR_RETURN,
	TIB_CHAR_ROUND,
	TIB_CHAR_SIN,
	TIB_CHAR_STOP,
	TIB_CHAR_STOREPIC,
	TIB_CHAR_TAN,
	TIB_CHAR_TEXT,
	TIB_CHAR_THEN,
	TIB_CHAR_TRANSPOSE,
	TIB_CHAR_WHILE,
	TIB_CHAR_XMAX,
	TIB_CHAR_XMIN,
	TIB_CHAR_XSCL,
	TIB_CHAR_YMAX,
	TIB_CHAR_YMIN,
	TIB_CHAR_YSCL,

	/* meta */
	TIB_FIRST_CHAR = 128,
	TIB_LAST_CHAR = TIB_CHAR_YSCL
};

const char *
tib_special_char_text(int c);

int
tib_encode_str(struct tib_expr *dest, const char *src);

int
tib_keyword_init(void);

void
tib_keyword_free(void);

#endif
