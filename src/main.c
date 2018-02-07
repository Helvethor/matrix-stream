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
    struct RGBLedMatrixOptions options;
    struct RGBLedMatrix * matrix;
    struct LedCanvas * canvas;
    
    int listen_socket, connection_socket;
    int flags;
    struct sockaddr_in server_addr;

    struct matrix_stream * stream;
    struct matrix_color color;
    uint32_t width, height;

    signal(SIGINT, interrupt_handler);

    options.hardware_mapping = "adafruit-hat";
    options.rows = 32;
    options.chain_length = 2;
    options.parallel = 1;
    options.pwm_bits = 11;
    options.pwm_lsb_nanoseconds = 130;
    options.brightness = 100;
    options.scan_mode = 0;
    options.led_rgb_sequence = NULL;
    options.disable_hardware_pulsing = 0;
    options.show_refresh_rate = 0;
    options.inverse_colors = 0;

    matrix = led_matrix_create_from_options(&options, NULL, NULL);;
    canvas= led_matrix_create_offscreen_canvas(matrix);
    led_canvas_get_size(canvas, &width, &height);

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
        die("Couldn't create server socket");

    // Set the listening socket to be non-blocking
    flags = fcntl(listen_socket, F_GETFL, 0);
    if (flags == -1)
        die("Couldn't get socket flags");
    flags |= O_NONBLOCK;
    if (fcntl(listen_socket, F_SETFL, flags) < 0)
        die("Couldn't set socket flags");

    // Set the listening socket ot be reusable
    int enable;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        die("Couldn't set SO_REUSEADDR on listening socket");
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
        die("Couldn't set SO_REUSEPORT on listening socket");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(31415);
    if (bind(listen_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die("Couldn't bind server socket");

    listen(listen_socket, 1);
    while (!stop) {
        connection_socket = accept(listen_socket, NULL, NULL);
        if (connection_socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            die("Failed to accept connection");
        }

        stream = matrix_stream_server_new(connection_socket, width, height);
        if (!stream)
            die("Couldn't create stream struct");

        while (!stop && matrix_stream_recv(stream)) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    matrix_stream_pixel_get(stream, x, y, &color);
                    led_canvas_set_pixel(canvas, x, y,
                        color.red, color.green, color.blue);
                }
            }
            canvas = led_matrix_swap_on_vsync(matrix, canvas);
        }
    }

    close(connection_socket);
    close(listen_socket);
    return 0;
}
