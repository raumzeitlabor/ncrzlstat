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

#define TIMEOUT		1000
#define LIST_SPACING	5

#define PV_GENERIC(X, Y, LBL, FMT, VAL, UNIT)	do {			\
	mvprintw((X), (Y), "%-10s", (LBL));				\
	attron(A_BOLD);							\
	printw((FMT), (VAL));						\
	attrset(A_NORMAL);						\
	addstr((UNIT));							\
} while (0)

#define PV_INTEGER(X, Y, LBL, VAL, UNIT)				\
	PV_GENERIC((X), (Y), (LBL), "%4d    ", (VAL), (UNIT))

#define PV_DOUBLE_2(X, Y, LBL, VAL, UNIT)				\
	PV_GENERIC((X), (Y), (LBL), "%7.2f ", (VAL), (UNIT))

#define PV_DOUBLE_0(X, Y, LBL, VAL, UNIT)				\
	PV_GENERIC((X), (Y), (LBL), "%4.0f    ", (VAL), (UNIT))

static void list_present(int y, int x, struct model *model);

void
ui_display(struct model *model)
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

	PV_INTEGER(2, 0, "Members:", model->members, "");
	PV_INTEGER(3, 0, "Present:", model->present, "");
	PV_INTEGER(4, 0, "Devices:", model->devices, "");

	PV_DOUBLE_2(2, 19, "Balance:", model->balance, "EUR");
	PV_DOUBLE_2(3, 19, "Temp:", model->temperature, "deg C");
	PV_DOUBLE_2(4, 19, "Latency:", model->latency, "ms");

	PV_DOUBLE_0(2, 47, "Drain:", model->drain, "W");
	PV_DOUBLE_0(3, 47, "Upload", model->upload, "kB/s");
	PV_DOUBLE_0(4, 47, "Download:", model->download, "kB/s");

	list_present(6, 0, model);

	refresh();
}

void
ui_init(void)
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
ui_deinit(void)
{
	endwin();
}

enum ui_event
ui_getevent(void)
{
	enum ui_event event;

#ifdef OLDCURSES
	/*
	 * This doupdate() is a workaround for a bug in older ncurses
	 * implementations (at least needed for vanilla ncurses 5.7). If the
	 * display won't redraw itself correctly after the terminal has been
	 * resized, try this workaround.
	 */
	doupdate();
#endif

	int ch = getch();

	switch (ch) {
	case 'q':
	case 'Q':
		event = UI_QUIT;
		break;
	case KEY_RESIZE:
		event = UI_RESIZE;
		break;
	default:
		event = UI_UNDEFINED;
		break;
	}

	return (event);
}

static void
list_present(int y, int x, struct model *model)
{
	assert(y >= 0);
	assert(x >= 0);
	assert(model != NULL);

	size_t xoff = 0;
	for (int i = 0; i < model->present; i++) {
		int yoff = (i % (LINES - y));
		int abs_y = yoff + y;
		if (i > 0 && yoff == 0) {
			x += xoff + LIST_SPACING;
			xoff = 0;
		}

		size_t namelen = strlen(model->presentnames[i]);
		if (xoff < namelen)
			xoff = namelen;

		mvprintw(abs_y, x, "%s", model->presentnames[i]);
	}
}
