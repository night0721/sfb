#include <arpa/inet.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

/* For slug generation */
const char *symbols = "abcdefghijklmnopqrstuvwxyz0123456789";
/* Permission for directories */
const int permission = S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH | S_IXGRP;

typedef struct {
	int socket;
	struct sockaddr_in address;
} connection;

void
die(char *err)
{
	fprintf(stderr, "%s\n", err);
	exit(1);
}

static void
error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf("[ERROR] ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}

static void
status(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf("[LOG] ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}

/*
 * Create string with current date and time
 */
static void
get_date(char *buf)
{
	time_t cur_time = time(NULL);

	/* Save data to provided buffer */
	if (ctime_r(&cur_time, buf) == NULL) {
		/* Couldn't get date, set first byte to 0 so it won't be displayed */
		buf[0] = 0;
		return;
	}
	/* Remove additional new line */
	buf[strlen(buf) - 1] = 0;
}

static void 
save_entry(const char *ip, const char *hostname, const char *id)
{
	FILE *f = fopen(log_file_path, "a");
	if (!f) {
		error("Error opening log file");
		return;
	}

	char date[26];
	get_date(date);

	/* Write request to file */
	fprintf(f, "[%s] [%s] [%s] [%s]\n", date, ip, hostname, id);
	fclose(f);
}

static void *
handle_connection(void *args)
{
	connection *c = (connection *) args;

	/* Get client's IP */
	const char *ip = inet_ntoa(c->address.sin_addr);

	/* Get client's hostname */
	char hostname[512];
	if (getnameinfo((struct sockaddr *)&c->address, sizeof(c->address),
			hostname, sizeof(hostname), NULL, 0, 0) != 0 ) {
		/* Couldn't resolve a hostname */
		strcpy(hostname, "N/A");
	}

	char date[26];
	get_date(date);
	status("[%s] [%s] [%s]: New connection", date, ip, hostname);

	uint8_t *buffer = malloc(max_file_size);
	if (!buffer) {
		error("Error allocating memory");
		goto quit;
	}
	memset(buffer, 0, max_file_size);

	const int r = recv(c->socket, buffer, max_file_size, 0);
	if (r <= 0) {
		/* TODO: log unsuccessful and rejected connections */
		error("No data received from the client");
		goto quit;
	} else {
		buffer[r] = 0;
		/* TODO: Check if requested was performed with a known protocol */

		/* Generate id */
		char id[id_len + 1];
		for (int i = 0; i < id_len; i++) {
			int n = rand() % strlen(symbols);
			id[i] = symbols[n];
		}
		id[id_len] = 0;

		/* 1 for slash and 1 for null */
		size_t len = strlen(output_dir) + id_len + 2;

		/* Create path */
		char path[len];
		snprintf(path, len, "%s/%s", output_dir, id);

		/* Try create directory for paste */
		if (mkdir(path, permission) != 0) {
			error("Error creating directory for paste");
			goto quit;
		}

		/* Save to file */
		const char *file_name = "index.txt";

		len += strlen(file_name) + 1;

		/* Construct path */
		char paste_path[len];
		snprintf(paste_path, len, "%s/%s", path, file_name);

		/* Save file */
		FILE *f = fopen(paste_path, "w");
		if (!f) {
			goto quit;
		}

		if (fprintf(f, "%s", buffer) < 0) {
			fclose(f);
			goto quit;
		}
		fclose(f);

		/* Write a response to the user */
		const size_t url_len = strlen(url) + id_len + 3;

		char full_url[url_len];
		snprintf(full_url, url_len, "%s/%s\n", url, id);

		/* Send the response */
		write(c->socket, full_url, url_len);
		status("[%s] [%s] [%s]: Received %d bytes, saved to %s", date, ip, hostname, r, id);
		save_entry(ip, hostname, id);
		goto quit;
	}
quit:
	free(buffer);
	close(c->socket);
	free(c);
	pthread_exit(NULL);
	return NULL;
}

int
main(void)
{
	srand(time(NULL));

	mkdir(output_dir, permission);
	if (access(output_dir, W_OK) != 0) {
		error("Output directory is not writable");
		return -1;
	}
	FILE *f = fopen(log_file_path, "a+");
	if (!f) {
		die("Cannot open log file");
	}
	fclose(f);
	
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	/* Reuse address */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1} , sizeof(int)) < 0) {
		perror("setsockopt");
		return -1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(listen_addr);
	address.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("bind");
		return -1;
	}

	if (listen(fd, 128) != 0) {
		perror("listen");
		return -1;
	}

	status("spb listening on %s:%d", listen_addr, port);

	while (1) {
		/* Create a thread for each connection */
		struct sockaddr_in address;
		socklen_t addlen = sizeof(address);

		/* Get client fd */
		int s = accept(fd, (struct sockaddr *) &address, &addlen);
		if (s < 0) {
			error("Error on accepting connection!");
			continue;
		}

		connection *c = malloc(sizeof(connection));
		if (!c) {
			die("Error allocating memory");
		}
		c->socket = s;
		c->address = address;

		pthread_t id;

		if (pthread_create(&id, NULL, &handle_connection, c) != 0) {
			error("Error spawning thread");
			continue;
		}
		pthread_detach(id);
	}
	return 0;
}
