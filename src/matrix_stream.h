#pragma(once)
#include <stdint.h>
#include <stdbool.h>

struct matrix_stream;
struct matrix_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct matrix_stream * matrix_stream_server_new(
    int socket, uint32_t width, uint32_t height);
struct matrix_stream * matrix_stream_client_new(
    int socket, uint32_t * width, uint32_t * height);
void matrix_stream_pixel_set(
    struct matrix_stream * stream, uint32_t x, uint32_t y,
    struct matrix_color * color);
void matrix_stream_pixel_get(
    struct matrix_stream * stream,
    uint32_t x, uint32_t y, struct matrix_color * color);
bool matrix_stream_recv(struct matrix_stream * stream);
bool matrix_stream_send(struct matrix_stream * stream);
