#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <led-matrix-c.h>
#include "matrix_stream.h"

static volatile bool stop = false;

void error(char * message) {
    if (errno != 0)
        fprintf(stderr, "%s: %s\n", message, strerror(errno));
    else
        fprintf(stderr, "%s\n", message);
}

void die(char * message) {
    error(message);
    exit(EXIT_FAILURE);
}

void interrupt_handler(int dummy) {
    stop = true;
}

int main(int argc, char** argv) {
    int client_socket;
    struct sockaddr_in server_addr;

    struct matrix_stream * stream;
    struct matrix_color color;
    uint32_t width, height;

    signal(SIGINT, interrupt_handler);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
        die("Couldn't create server socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(31415);
    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die("Couldn't connect to server");

    stream = matrix_stream_client_new(client_socket, &width, &height);
    if (!stream)
        die("Couldn't create stream struct");

    while (!stop) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                color.red = x + y;
                color.green = 2 * x - y;
                color.blue = x * y;
                matrix_stream_pixel_set(stream, x, y, &color);
            }
        }
        matrix_stream_send(stream);
    }

    close(client_socket);

    return 0;
}
