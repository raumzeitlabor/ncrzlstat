/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 *  Ported to Linux with curl instead of fetch by <don4221@gmail.com>
 *  Marco "don" Kaulea
 */

#include <assert.h>
#include <curses.h>
#include <errno.h>
#include <jansson.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/param.h>
#include <stdio.h>
#include <curl/curl.h>

#define STATUSURL	"http://status.raumzeitlabor.de/api/full.json"
#define COSMURL		"http://api.cosm.com/v2/feeds/42055.json?key=%s"

struct model {
	/* status */
	bool door;
	int devices;
	int present;
	time_t time;
	char **presentnames;
	/* cosm */
	int members;
	double balance;
	double temperature;
	double drain;
	double latency;
	double download;
	double upload;
};

void		 usage(void);
FILE		*fetch(const char *url);
char        *fetch_data_string(const char *url);
struct model	*parse_model(char *status, char *cosm);
void		 free_model(struct model *model);
void		 init_curses(void);
void		 deinit_curses(void);
void		 display(struct model *model);
int		 namecmp(const void *name1, const void *name2);
void		 parse_model_status(struct model *model, char *status);
void		 parse_model_cosm(struct model *model, char *cosm);

int
main(int argc, char *argv[])
{
	int ch;
	while ((ch = getopt(argc, argv, "h")) != -1) {
		switch (ch) {
		case 'h':
			usage();
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

	init_curses();
	atexit(&deinit_curses);

	do {
        char *status = fetch_data_string(STATUSURL);
		assert(status != NULL);

        char *cosm = fetch_data_string(cosmurl);
		assert(cosm != NULL);

		struct model *model = parse_model(status, cosm);
		assert(model != NULL);

		display(model);

		free(cosm);
		free(status);
		free_model(model);

	} while (getch() != 'q');

	free(cosmurl);

	return (0);
}

void
display(struct model *model)
{
	clear();

	attrset(A_NORMAL);
	mvaddstr(0, 0, "RaumZeitStatus");
	move(0, COLS - 24);
	addstr(ctime(&(model->time)));

	mvaddstr(2, 0, "Status:  ");
	attron(A_BOLD);
	if (model->door) {
		attron(COLOR_PAIR(1));
		addstr("open");
	} else {
		attron(COLOR_PAIR(2));
		addstr("closed");
	}
	attrset(A_NORMAL);

	mvaddstr(3, 0, "Devices: ");
	attron(A_BOLD);
	printw("%d", model->devices);
	attrset(A_NORMAL);

	mvaddstr(4, 0, "Present: ");
	attron(A_BOLD);
	printw("%d", model->present);
	attrset(A_NORMAL);

	mvaddstr(2, 20, "Members: ");
	attron(A_BOLD);
	printw("%d", model->members);
	attrset(A_NORMAL);

	mvaddstr(3, 20, "Balance: ");
	attron(A_BOLD);
	printw("%.2f ", model->balance);
	attrset(A_NORMAL);

	mvaddstr(4, 20, "Temp:    ");
	attron(A_BOLD);
	printw("%.1f ", model->temperature);
	attrset(A_NORMAL);

	mvaddstr(2, 40, "Drain:   ");
	attron(A_BOLD);
	printw("%.0f", model->drain);
	attrset(A_NORMAL);

	mvaddstr(3, 40, "Latency: ");
	attron(A_BOLD);
	printw("%.3f", model->latency);
	attrset(A_NORMAL);

	mvaddstr(4, 40, "Up/Down: ");
	attron(A_BOLD);
	printw("%.0f", model->upload);
	attrset(A_NORMAL);
	addstr("/");
	attron(A_BOLD);
	printw("%.0f", model->download);
	attrset(A_NORMAL);

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
}

void
init_curses(void)
{
	initscr();
	cbreak();
	noecho();
	timeout(10000);

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

struct model *
parse_model(char *status, char *cosm)
{
	assert(status != NULL);
	assert(cosm != NULL);

	struct model *model = malloc(sizeof(struct model));
	if (model == NULL) {
		fprintf(stderr, "Could not allocate memory for model: %s\n",
		    strerror(errno));
		exit(EXIT_FAILURE);
	}

	model->time = time(NULL);

	parse_model_status(model, status);
	parse_model_cosm(model, cosm);

	return (model);
}

void
parse_model_status(struct model *model, char *status)
{
	json_error_t error;
	json_t *json = json_loads(status, 0, &error);
	if (json == NULL) {
		fprintf(stderr, "Could not parse status: %s\n", error.text);
		fprintf(stderr, status);
        exit(EXIT_FAILURE);
	}

	json_t *details = json_object_get(json, "details");
	if (!json_is_object(details)) {
		fprintf(stderr, "details is not an object\n");
		exit(EXIT_FAILURE);
	}

	json_t *obj;
	obj = json_object_get(details, "tuer");
	if (!json_is_string(obj)) {
		fprintf(stderr, "tuer is not a string\n");
		exit(EXIT_FAILURE);
	}
	model->door = strcmp("1", json_string_value(obj)) == 0 ? true : false;

	obj = json_object_get(details, "geraete");
	if (!json_is_integer(obj)) {
		fprintf(stderr, "geraete is not an integer\n");
		exit(EXIT_FAILURE);
	}
	model->devices = json_integer_value(obj);

	obj = json_object_get(details, "laboranten");
	if (!json_is_array(obj)) {
		fprintf(stderr, "laboranten is not an array\n");
		exit(EXIT_FAILURE);
	}
	model->present = json_array_size(obj);

	model->presentnames = malloc(model->present * sizeof(char*));
	if (model->presentnames == NULL) {
		fprintf(stderr, "Could not allocate presentnames: %s\n",
		    strerror(errno));
	}
	for (int i = 0; i < model->present; i++) {
		json_t *m = json_array_get(obj, i);
		if (!json_is_string(m)) {
			fprintf(stderr, "member is not a string\n");
			exit(EXIT_FAILURE);
		}
		model->presentnames[i] = strdup(json_string_value(m));
		if (model->presentnames[i] == NULL) {
			fprintf(stderr, "Could not strdup knownmember: %s\n",
			    strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	qsort(model->presentnames, model->present, sizeof(char**), &namecmp);

	json_decref(json);
}

void
parse_model_cosm(struct model *model, char *cosm)
{
	json_error_t error;
	json_t *json = json_loads(cosm, 0, &error);
	if (json == NULL) {
		fprintf(stderr, "Could not parse cosm: %s\n", error.text);
        fprintf(stderr, cosm);
		exit(EXIT_FAILURE);
	}

	json_t *datastreams = json_object_get(json, "datastreams");
	if (!json_is_array(datastreams)) {
		fprintf(stderr, "datastreams is not an array\n");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < json_array_size(datastreams); i++) {
		json_t *stream = json_array_get(datastreams, i);
		if (!json_is_object(stream)) {
			fprintf(stderr, "stream is not an object\n");
			exit(EXIT_FAILURE);
		}

		json_t *id = json_object_get(stream, "id");
		if (!json_is_string(id)) {
			fprintf(stderr, "id is not a string\n");
			exit(EXIT_FAILURE);
		}

		if (strcmp("Mitglieder", json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->members = (int)strtod(json_string_value(val),
			    NULL);
		}

		if (strcmp("Kontostand", json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->balance = strtod(json_string_value(val), NULL);
		}

		if (strcmp("Temperatur_Raum_Beamerplattform",
		    json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->temperature = strtod(json_string_value(val),
			    NULL);
		}

		if (strcmp("Strom_Leistung", json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->drain = strtod(json_string_value(val), NULL);
		}

		if (strcmp("internet.latency", json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->latency = strtod(json_string_value(val), NULL);
		}

		if (strcmp("internet.upload", json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->upload = strtod(json_string_value(val), NULL);
		}

		if (strcmp("internet.download", json_string_value(id)) == 0) {
			json_t *val = json_object_get(stream, "current_value");
			if (!json_is_string(val)) {
				fprintf(stderr, "value is not a string\n");
				exit(EXIT_FAILURE);
			}
			model->download = strtod(json_string_value(val), NULL);
		}
	}

	json_decref(json);
}

void
free_model(struct model *model)
{
	assert(model != NULL);

	for (int i = 0; i < model->present; i++) {
		free(model->presentnames[i]);
	}
	free(model->presentnames);

	free(model);
}

FILE *
fetch(const char *url)
{
	assert(url != NULL);
/*
	FILE *f = fetchGetURL(url, "");
	if (f == NULL) {
		fprintf(stderr, "Could not get URL \"%s\"\n", url);
		exit(EXIT_FAILURE);
	}

	return (f);
*/
    return NULL;
}

int
curl_writer(char *data, size_t size, size_t nmemb,
       char **buffer)
{
    int result = 0;

    if (buffer != NULL)
    {
        //buffer->append(data,size * nmemb);
        *buffer = realloc(*buffer, strlen(*buffer) + 1 + size * nmemb);
        //char* tmp = malloc(size*nmemb+1);
        //tmp[size*nmemb] = 0;
        strncat(*buffer, data, size * nmemb);
        result = size * nmemb;
    }

   return result;
}

char *
fetch_data_string(const char *url)
{
    CURL *curl;
    CURLcode result;

    char* buffer = malloc(sizeof(char)*300);
    buffer[0] = '\0';
    //create curl handle
    curl = curl_easy_init();
    assert(curl != NULL);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    result = curl_easy_perform(curl);

    curl_easy_cleanup;

    char* ret = malloc(strlen(buffer)+1);
    strcpy(ret, buffer);
    free(buffer);
    return ret;
}

void
usage(void)
{
	fprintf(stderr, "ncrzlstat has no options.\n");//getprogname());
}

int
namecmp(const void *name1, const void *name2)
{
	const char *c1 = *(char * const *)name1;
	const char *c2 = *(char * const *)name2;

	return strcmp(c1, c2);
}
