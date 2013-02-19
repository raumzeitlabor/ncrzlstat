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
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ncrzlstat.h"
#include "ui.h"
#include "fetch.h"
#include "parse.h"

#define STATUSURL	"https://status.raumzeitlabor.de/api/full.json"
#define COSMURL		"https://api.cosm.com/v2/feeds/42055.json?key=%s"

void	usage(void);

int
main(int argc, char *argv[])
{
	enum fetch_ipversion ipresolve = FETCH_IPVANY;

	int ch;
	while ((ch = getopt(argc, argv, "h46")) != -1) {
		switch (ch) {
		case '4':
			if (ipresolve == FETCH_IPV6ONLY) {
				usage();
				exit(EXIT_FAILURE);
			}
			ipresolve = FETCH_IPV4ONLY;
			break;
		case '6':
			if (ipresolve == FETCH_IPV4ONLY) {
				usage();
				exit(EXIT_FAILURE);
			}
			ipresolve = FETCH_IPV6ONLY;
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	char *cosmkey = getenv("RZLCOSMKEY");
	if (cosmkey == NULL) {
		fprintf(stderr,
		    "Environment variable RZLCOSMKEY is not set.\n");
		exit(EXIT_FAILURE);
	}
	char *cosmurl;
	asprintf(&cosmurl, COSMURL, cosmkey);

	ui_init();
	atexit(&ui_deinit);

	enum ui_event event = UI_UNDEFINED;
	while (event != UI_QUIT) {
		char *status = fetch_data_string(STATUSURL, ipresolve);
		assert(status != NULL);

		char *cosm = fetch_data_string(cosmurl, ipresolve);
		assert(cosm != NULL);

		bool loop = true;
		bool refresh = true;
		time_t last = time(NULL);
		while (loop) {
			if (refresh) {
				refresh = false;

				struct model *model = parse_fill_model(last,
				    status, cosm);
				assert(model != NULL);

				ui_display(model);

				parse_free_model(model);
			}

			event = ui_getevent();
			switch (event) {
			case UI_QUIT:
				loop = false;
				break;
			case UI_RESIZE:
				refresh = true;
				break;
			case UI_UNDEFINED:
			default:
				break;
			}

			if (last <= time(NULL) - REFRESH)
				loop = false;
		}

		free(cosm);
		free(status);
	}

	free(cosmurl);

	return (0);
}

void
usage(void)
{
	fprintf(stderr, "usage: ncrzlstat [-h] [-4|-6]\n");
}
