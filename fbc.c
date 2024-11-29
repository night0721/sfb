#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "config.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
	 /* Determine file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(file_size);
    if (!buffer) {
        perror("malloc");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("fread");
        free(buffer);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    struct sockaddr_in addr;
    char response[512];

    /* Resolve domain to IP */
	struct hostent *server = gethostbyname(domain);
	if (server == NULL) {
		fprintf(stderr, "No such host %s", domain);
	}

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
	memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, buffer, file_size, 0) < 0) {
        perror("send");
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    free(buffer);

    memset(response, 0, sizeof(response));
    ssize_t bytes_received = recv(sockfd, response, sizeof(response) - 1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("%s", response);
    close(sockfd);
    return 0;
}
