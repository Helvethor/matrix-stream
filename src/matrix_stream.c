#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "matrix_stream.h"

struct matrix_stream {
    int socket;
    uint32_t width;
    uint32_t height;
    struct matrix_color * buffer;
};


uint32_t buffer_size(struct matrix_stream * stream)
{
    return sizeof(struct matrix_color) * stream->width * stream->height;
}


struct matrix_color * buffer_offset(struct matrix_stream * stream, uint32_t x, uint32_t y)
{
    return stream->buffer + (x + stream->width * y);
}


void serialize_uint32_t(uint8_t * buffer, uint32_t value)
{
    value = htonl(value);
    memcpy(buffer, &value, 4);
}


void deserialize_uint32_t(uint8_t * buffer, uint32_t * value)
{
    memcpy(value, buffer, 4);
    *value = ntohl(*value);
}


void send_dimension(struct matrix_stream * stream)
{
    uint8_t buffer[8];
    serialize_uint32_t(buffer, stream->width);
    serialize_uint32_t(buffer + 4, stream->height);
    write(stream->socket, buffer, 8);
}


void recv_dimension(struct matrix_stream * stream) {
    uint8_t buffer[8];
    read(stream->socket, buffer, 8);
    deserialize_uint32_t(buffer, &stream->width);
    deserialize_uint32_t(buffer + 4, &stream->height);
}


struct matrix_stream * matrix_stream_server_new(
    int socket, uint32_t width, uint32_t height)
{
    struct matrix_stream * stream;

    stream = malloc(sizeof(struct matrix_stream));
    if (!stream)
        return NULL;

    stream->socket = socket;
    stream->width = width;
    stream->height = height;
    stream->buffer = malloc(buffer_size(stream));

    if (!stream->buffer) {
        free(stream);
        return NULL;
    }

    send_dimension(stream);

    return stream;
}


struct matrix_stream * matrix_stream_client_new(
    int socket, uint32_t * width, uint32_t * height)
{
    struct matrix_stream * stream;

    stream = malloc(sizeof(struct matrix_stream));
    if (!stream)
        return NULL;

    stream->socket = socket;
    recv_dimension(stream);

    *width = stream->width;
    *height = stream->height;

    stream->buffer = malloc(buffer_size(stream));
    if (!stream->buffer) {
        free(stream);
        return NULL;
    }

    return stream;
}


void matrix_stream_pixel_set(
    struct matrix_stream * stream, uint32_t x, uint32_t y,
    struct matrix_color * color)
{
    memcpy(buffer_offset(stream, x, y), color, sizeof(struct matrix_color));
}


void matrix_stream_pixel_get(
    struct matrix_stream * stream, uint32_t x, uint32_t y,
    struct matrix_color * color)
{
    memcpy(color, buffer_offset(stream, x, y), sizeof(struct matrix_color));
}


bool matrix_stream_recv(struct matrix_stream * stream)
{
    uint32_t size = buffer_size(stream);
    if (read(stream->socket, stream->buffer, size) != size)
        return false;
    return true;
}


bool matrix_stream_send(struct matrix_stream * stream)
{
    uint32_t size = buffer_size(stream);
    if (write(stream->socket, stream->buffer, buffer_size(stream)) != size)
        return false;
    return true;
}
