/*
 * Copyright (c) 2023 Wilhelm Wiens <wilhelmwiens@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef HAVE_CONFIG_H
#include <jansson_private_config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "jansson_private.h"
#include "ringbuffer.h"
#include <jansson_config.h> /* for JSON_INLINE */

#define RINGBUFFER_MIN_BUFFER_SIZE 8

void ringbuffer_init(ringbuffer_t *ringbuffer) {
    ringbuffer->buffer_size = 0;
    ringbuffer->start = 0;
    ringbuffer->elementCount = 0;
    ringbuffer->buffer = NULL;
}

void ringbuffer_close(ringbuffer_t *ringbuffer) { ringbuffer_clear(ringbuffer); }

int ringbuffer_set(ringbuffer_t *ringbuffer, size_t index, json_t *value) {
    json_t **j;

    if (index >= ringbuffer->elementCount) {
        return -1;
    }

    j = &(ringbuffer->buffer[(ringbuffer->start + index) % ringbuffer->buffer_size]);
    json_decref(*j);
    *j = value;

    return 0;
}

/**
 * Move content from scr to dest. Takes care of overlap. Input is virtual position.
 * @param ringbuffer ringbuffer to work on
 * @param dest index of destination
 * @param src index of source
 * @param count count of element to move
 */
static void ringbuffer_move(ringbuffer_t *ringbuffer, size_t dest, size_t src,
                            size_t count) {
    if (dest < src) {
        json_t **posRead =
            &(ringbuffer->buffer[(ringbuffer->start + src) % ringbuffer->buffer_size]);
        json_t **posWrite =
            &(ringbuffer->buffer[(ringbuffer->start + dest) % ringbuffer->buffer_size]);
        json_t **endOfBuffer = &(ringbuffer->buffer[ringbuffer->buffer_size]);

        do {
            size_t read_count = endOfBuffer - posRead;
            size_t write_count = endOfBuffer - posWrite;

            size_t resulting_count = min(read_count, write_count);
            resulting_count = min(resulting_count, count);

            memmove(posWrite, posRead, resulting_count * sizeof(json_t *));
            posWrite += resulting_count;
            posRead += resulting_count;
            if (posWrite == endOfBuffer)
                posWrite =
                    &(ringbuffer->buffer[(ringbuffer->start) % ringbuffer->buffer_size]);
            if (posRead == endOfBuffer)
                posRead =
                    &(ringbuffer->buffer[(ringbuffer->start) % ringbuffer->buffer_size]);
            count -= resulting_count;
        } while (count);
    } else {
        json_t **posRead;
        json_t **posWrite;
        json_t **startOfBuffer;
        dest += count - 1;
        src += count - 1;
        posRead =
            &(ringbuffer->buffer[(ringbuffer->start + src) % ringbuffer->buffer_size]);
        posWrite =
            &(ringbuffer->buffer[(ringbuffer->start + dest) % ringbuffer->buffer_size]);
        startOfBuffer = &(ringbuffer->buffer[0]);

        do {
            size_t read_count = posRead - startOfBuffer;
            size_t write_count = posWrite - startOfBuffer;

            size_t resulting_count = min(read_count, write_count);
            resulting_count = min(resulting_count, count);

            memmove(posWrite, posRead, resulting_count * sizeof(json_t *));
            posWrite -= resulting_count;
            posRead -= resulting_count;
            if (posWrite == startOfBuffer)
                posWrite = &(
                    ringbuffer->buffer[(ringbuffer->start + ringbuffer->buffer_size - 1) %
                                       ringbuffer->buffer_size]);
            if (posRead == startOfBuffer)
                posRead = &(
                    ringbuffer->buffer[(ringbuffer->start + ringbuffer->buffer_size - 1) %
                                       ringbuffer->buffer_size]);
            count -= resulting_count;
        } while (count);
    }
}

/**
 * Resize buffer
 * @param ringbuffer ringbuffer to work on
 * @param newSize new size of ringbuffer
 * @return 0 on success -1 on error (size to small to fit current data, allocation error)
 */
static int ringbuffer_resize(ringbuffer_t *ringbuffer, size_t newSize) {

    if (newSize < RINGBUFFER_MIN_BUFFER_SIZE) {
        newSize = RINGBUFFER_MIN_BUFFER_SIZE;
    }

    if (ringbuffer->buffer == NULL) { // optimize the initial case
        ringbuffer->buffer = jsonp_malloc(newSize * sizeof(json_t *));
        if (ringbuffer->buffer == NULL) {
            return -1;
        }
        ringbuffer->buffer_size = newSize;
        return 0;
    }

    if (newSize < ringbuffer->elementCount) {
        return -1;
    }

    {
        json_t **newBuffer =
            jsonp_realloc(ringbuffer->buffer, ringbuffer->buffer_size * sizeof(json_t *),
                          newSize * sizeof(json_t *));
        if (newBuffer == NULL) {
            return -1;
        }
        ringbuffer->buffer = newBuffer;
    }

    if (ringbuffer->start >
        ringbuffer->buffer_size - ringbuffer->elementCount) { // data is wrapping over
        const size_t upperBlockSize = ringbuffer->buffer_size - ringbuffer->start;
        const size_t lowerBlockSize =
            (ringbuffer->start + ringbuffer->elementCount) % ringbuffer->buffer_size;
        if (upperBlockSize < lowerBlockSize) { // less to move when moving start and up
            memmove(ringbuffer->buffer + newSize - upperBlockSize,
                    ringbuffer->buffer + ringbuffer->buffer_size - upperBlockSize,
                    upperBlockSize * sizeof(json_t *));
            ringbuffer->start = newSize - upperBlockSize;
        } else { // less to move when moving end and down
            const size_t newSpaceGrowth = newSize - ringbuffer->buffer_size;
            if (lowerBlockSize > newSpaceGrowth) { // to little space at the end of array
                memmove(ringbuffer->buffer + ringbuffer->buffer_size, ringbuffer->buffer,
                        newSpaceGrowth * sizeof(json_t *));
                memmove(ringbuffer->buffer, ringbuffer->buffer + newSpaceGrowth,
                        (lowerBlockSize - newSpaceGrowth) * sizeof(json_t *));
            } else {
                memmove(ringbuffer->buffer + ringbuffer->buffer_size, ringbuffer->buffer,
                        lowerBlockSize * sizeof(json_t *));
            }
        }
    }
    ringbuffer->buffer_size = newSize;

    return 0;
}

int ringbuffer_insert(ringbuffer_t *ringbuffer, size_t index, json_t *value) {
    if (index > ringbuffer->elementCount) {
        return -1;
    }

    if (ringbuffer->elementCount == ringbuffer->buffer_size) {
        if (ringbuffer_resize(ringbuffer, ringbuffer->buffer_size * 2)) {
            return -1;
        }
    }

    if (index == ringbuffer->elementCount) {
        ringbuffer->buffer[(ringbuffer->start + ringbuffer->elementCount) %
                           ringbuffer->buffer_size] = value;
    } else if (index == 0) {
        ringbuffer->start =
            (ringbuffer->start + ringbuffer->buffer_size - 1) % ringbuffer->buffer_size;
        ringbuffer->buffer[ringbuffer->start] = value;
    } else if (index <= ringbuffer->elementCount / 2) {
        ringbuffer->start =
            (ringbuffer->start + ringbuffer->buffer_size - 1) % ringbuffer->buffer_size;
        ringbuffer_move(ringbuffer, 0, 1, index);
        ringbuffer->buffer[(ringbuffer->start + index) % ringbuffer->buffer_size] = value;
    } else {
        ringbuffer_move(ringbuffer, index + 1, index, ringbuffer->elementCount - index);
        ringbuffer->buffer[(ringbuffer->start + index) % ringbuffer->buffer_size] = value;
    }
    ringbuffer->elementCount++;

    return 0;
}

int ringbuffer_append(ringbuffer_t *ringbuffer, json_t *value) {
    return ringbuffer_insert(ringbuffer, ringbuffer->elementCount, value);
}

int ringbuffer_append_ringbuffer(ringbuffer_t *ringbuffer, ringbuffer_t *other) {
    size_t i;
    size_t countAdded = 0;
    int error = 0;

    for (i = 0; i < other->elementCount && !error; i++) {
        error |= ringbuffer_append(ringbuffer, ringbuffer_get(other, i));
        countAdded++;
    }

    if (error) {
        for (i = 0; i < countAdded; i++)
            ringbuffer_del(ringbuffer, ringbuffer->elementCount - 1);
    }

    return error;
}

json_t *ringbuffer_get(ringbuffer_t *ringbuffer, size_t index) {
    if (index >= ringbuffer->elementCount) {
        return NULL;
    }

    return ringbuffer->buffer[(ringbuffer->start + index) % ringbuffer->buffer_size];
}

int ringbuffer_del(ringbuffer_t *ringbuffer, size_t index) {
    if (index >= ringbuffer->elementCount) {
        return -1;
    }

    json_decref(
        ringbuffer->buffer[(ringbuffer->start + index) % ringbuffer->buffer_size]);
    if (index == ringbuffer->elementCount - 1) {
    } else if (index == 0) {
        ringbuffer->start = (ringbuffer->start + 1) % ringbuffer->buffer_size;
    } else if (index < ringbuffer->elementCount / 2) {
        ringbuffer_move(ringbuffer, 1, 0, index);
        ringbuffer->start = (ringbuffer->start + 1) % ringbuffer->buffer_size;
    } else {
        ringbuffer_move(ringbuffer, index, index + 1, ringbuffer->elementCount - index);
    }
    ringbuffer->elementCount--;

    if (ringbuffer->elementCount < ringbuffer->buffer_size / 8) {
        ringbuffer_resize(ringbuffer, ringbuffer->buffer_size / 2);
    }

    return 0;
}

void ringbuffer_clear(ringbuffer_t *ringbuffer) {
    size_t i;

    for (i = 0; i < ringbuffer->elementCount; i++) {
        json_decref(ringbuffer_get(ringbuffer, i));
    }

    jsonp_free(ringbuffer->buffer);
    ringbuffer->buffer = NULL;
    ringbuffer->buffer_size = 0;
    ringbuffer->elementCount = 0;
    ringbuffer->start = 0;
}
