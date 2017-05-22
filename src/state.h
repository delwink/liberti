/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016-2017 Delwink, LLC
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

#ifndef DELWINK_LIBERTI_STATE_H
#define DELWINK_LIBERTI_STATE_H

#include "tibexpr.h"
#include "tibtype.h"

#define MAX_HISTORY 35

enum action_state
{
	STATE_NORMAL,
	STATE_2ND,
	STATE_ALPHA,
	NUM_ACTION_STATES
};

struct history
{
	struct tib_expr entry;
	struct tib_expr answer_string;

	TIB *answer;
};

struct state
{
	struct history history[MAX_HISTORY];
	struct tib_expr entry;

	enum action_state action_state;
	int entry_cursor;
	unsigned int history_len;

	bool blink_state;
	bool insert_mode;
};

int
load_state(struct state *dest, const char *path);

int
save_state(const struct state *state, const char *path);

void
state_destroy(struct state *state);

void
entry_move_cursor(struct state *state, int distance);

int
entry_write(struct state *state, int c);

int
entry_recall(struct state *state);

void
change_action_state(struct state *state, enum action_state action_state);

int
state_calc_entry(struct state *state);

int
state_add_history(struct state *state, const struct tib_expr *in,
		const TIB *answer);

void
state_clear_history(struct state *state);

#endif
