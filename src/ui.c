/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 *
 *  Ported to Linux with curl instead of fetch by <DonMarco42@gmail.com>
 *  Marco "don" Kaulea. He prefers Single Malt Scotch.
 */

#define _GNU_SOURCE

#include <assert.h>
#include <curses.h>
#include <string.h>
#include <time.h>

#include "ncrzlstat.h"
#include "ui.h"

#define TIMEOUT		10000

int
display(struct model *model)
{
	assert(model != NULL);

	clear();

	attrset(A_NORMAL);
	mvaddstr(0, 0, "RaumZeitStatus ");
	if (model->door) {
		attron(COLOR_PAIR(1));
		printw("%6s", "[open]");
	} else {
		attron(COLOR_PAIR(2));
		printw("%6s", "[closed]");
	}
	attrset(A_NORMAL);

	move(0, COLS - 24);
	addstr(ctime(&(model->time)));

	mvprintw(2, 0, "%-9s", "Members:");
	attron(A_BOLD);
	printw("%3d", model->members);
	attrset(A_NORMAL);

	mvprintw(3, 0, "%-9s", "Present:");
	attron(A_BOLD);
	printw("%3d", model->present);
	attrset(A_NORMAL);

	mvprintw(4, 0, "%-9s", "Devices:");
	attron(A_BOLD);
	printw("%3d", model->devices);
	attrset(A_NORMAL);

	mvprintw(2, 17, "%-13s",  "Balance:");
	attron(A_BOLD);
	printw("%7.2f ", model->balance);
	attrset(A_NORMAL);
	addstr("EUR");

	mvprintw(3, 17, "%-13s", "Temperature:");
	attron(A_BOLD);
	printw("%7.2f ", model->temperature);
	attrset(A_NORMAL);
	addstr("deg C");

	mvprintw(4, 17, "%-13s", "Latency:");
	attron(A_BOLD);
	printw("%7.2f ", model->latency);
	attrset(A_NORMAL);
	addstr("ms");

	mvprintw(2, 48, "%-13s", "Power Drain:");
	attron(A_BOLD);
	printw("%4.0f ", model->drain);
	attrset(A_NORMAL);
	addstr("W");

	mvprintw(3, 48, "%-13s", "Upload:");
	attron(A_BOLD);
	printw("%4.0f ", model->upload);
	attrset(A_NORMAL);
	addstr("kB/s");

	mvprintw(4, 48, "%-13s", "Download:");
	attron(A_BOLD);
	printw("%4.0f ", model->download);
	attrset(A_NORMAL);
	addstr("kB/s");

	int x = 0;
	size_t xoff = 0;
	for (int i = 0; i < model->present; i++) {
		int yoff = (i % (LINES - 6));
		int y = yoff + 6;
		if (i > 0 && yoff == 0)
			x += xoff + 2;

		size_t namelen = strlen(model->presentnames[i]);
		if (xoff < namelen)
			xoff = namelen;

		mvprintw(y, x, "%s", model->presentnames[i]);
	}

	refresh();

	return getch();
}

void
init_curses(void)
{
	initscr();
	cbreak();
	noecho();
	timeout(TIMEOUT);

	if (has_colors()) {
		start_color();

		init_pair(1, COLOR_GREEN, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
	}
}

void
deinit_curses(void)
{
	endwin();
}


