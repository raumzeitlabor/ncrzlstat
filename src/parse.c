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
#include <errno.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ncrzlstat.h"
#include "parse.h"

static int	namecmp(const void *name1, const void *name2);
static void	parse_model_cosm(struct model *model, char *cosm);
static void	parse_model_status(struct model *model, char *status);

struct model *
parse_fill_model(char *status, char *cosm)
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
parse_free_model(struct model *model)
{
	assert(model != NULL);

	for (int i = 0; i < model->present; i++) {
		free(model->presentnames[i]);
	}
	free(model->presentnames);

	free(model);
}

static void
parse_model_status(struct model *model, char *status)
{
	assert(model != NULL);
	assert(status != NULL);

	json_error_t error;
	json_t *json = json_loads(status, 0, &error);
	if (json == NULL) {
		fprintf(stderr, "Could not parse status: %s\n", error.text);
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

static void
parse_model_cosm(struct model *model, char *cosm)
{
	assert(model != NULL);
	assert(cosm != NULL);

	json_error_t error;
	json_t *json = json_loads(cosm, 0, &error);
	if (json == NULL) {
		fprintf(stderr, "Could not parse cosm: %s\n", error.text);
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

static int
namecmp(const void *name1, const void *name2)
{
	assert(name1 != NULL);
	assert(name2 != NULL);

	const char *c1 = *(char * const *)name1;
	const char *c2 = *(char * const *)name2;

	return strcmp(c1, c2);
}
