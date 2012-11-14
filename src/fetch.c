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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "fetch.h"

struct curl_write_buffer {
	size_t malloced;
	size_t used;
	char *buffer;
};

static int	curl_writer(char *data, size_t size, size_t nmemb,
		    struct curl_write_buffer *buffer);

char *
fetch_data_string(const char *url, enum fetch_ipversion ipresolve)
{
	assert(url != NULL);

	struct curl_write_buffer buffer = {
		.malloced = 0,
		.used = 0,
		.buffer = NULL,
	};

	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		fprintf(stderr, "Could not initialize curl\n");
		exit(EXIT_FAILURE);
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

	switch (ipresolve){
	case (FETCH_IPV4ONLY):
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
		break;
	case (FETCH_IPV6ONLY):
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
		break;
	case (FETCH_IPVANY):
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE,
		    CURL_IPRESOLVE_WHATEVER);
		break;
	}

	CURLcode result = curl_easy_perform(curl);
	if (result != 0) {
		fprintf(stderr, "Could not fetch URL: %s\n", url);
		exit(EXIT_FAILURE);
	}

	curl_easy_cleanup(curl);

	char *data = strdup(buffer.buffer);
	if (data == NULL) {
		fprintf(stderr, "Could not strdup buffer: %s\n",
		    strerror(errno));
		exit(EXIT_FAILURE);
	}

	free(buffer.buffer);

	return (data);
}

static int
curl_writer(char *data, size_t size, size_t nmemb,
    struct curl_write_buffer *buffer)
{
	assert(data != NULL);
	assert(size > 0);
	assert(nmemb > 0);
	assert(buffer != NULL);

	size_t total = size * nmemb;

	if (buffer->malloced == 0) {
		buffer->malloced = (total * 2) + 1;
		buffer->buffer = malloc(buffer->malloced);
		if (buffer->buffer == NULL) {
			fprintf(stderr, "Could not malloc buffer: %s\n",
			    strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (buffer->malloced < (buffer->used + total + 1)) {
		buffer->malloced += (total * 2) + 1;
		buffer->buffer = realloc(buffer->buffer, buffer->malloced);
		if (buffer->buffer == NULL) {
			fprintf(stderr, "Could not realloc buffer: %s\n",
			    strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	assert(buffer != NULL);

	memcpy(buffer->buffer + buffer->used, data, total);
	buffer->used += total;
	buffer->buffer[buffer->used] = '\0';

	return (total);
}
