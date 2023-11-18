/*
 * Copyright (c) 2023 Wilhelm Wiens <wilhelmwiens@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdlib.h>

#include "jansson.h"

typedef struct ringbuffer {
    size_t buffer_size;  // Allocated memory size in (json*)
    size_t start;        // Points on first element in buffer
    size_t elementCount; // Elements in ringbuffer
    json_t **buffer;
} ringbuffer_t;

/**
 * ringbuffer_init - Initialize a ringbuffer object
 *
 * @ringbuffer: The (statically allocated) ringbuffer object
 *
 * Initializes a statically allocated ringbuffer object. The object
 * should be cleared with ringbuffer_close when it's no longer used.
 */
void ringbuffer_init(ringbuffer_t *ringbuffer);

/**
 * ringbuffer_close - Release all resources used by a ringbuffer object
 *
 * @ringbuffer: The ringbuffer
 *
 * Destroys a statically allocated ringbuffer object.
 */
void ringbuffer_close(ringbuffer_t *ringbuffer);

/**
 * ringbuffer_set - Modify value in ringbuffer
 *
 * @ringbuffer: The ringbuffer object
 * @index: The index
 * @value: The value
 *
 * The value at given index is replaced with the new value. Value
 * is "stolen" in the sense that ringbuffer doesn't increment its
 * refcount but decreases the refcount when the value is no longer
 * needed.
 *
 * Returns 0 on success, -1 on failure (out of range).
 */
int ringbuffer_set(ringbuffer_t *ringbuffer, size_t index, json_t *value);

/**
 * ringbuffer_insert - Add value in ringbuffer
 *
 * @ringbuffer: The ringbuffer object
 * @index: The index
 * @value: The value
 *
 * The value is inserted at given index. Value is "stolen" in the
 * sense that ringbuffer doesn't increment its refcount but decreases
 * the refcount when the value is no longer needed.
 *
 * Returns 0 on success, -1 on failure (out of range).
 */
int ringbuffer_insert(ringbuffer_t *ringbuffer, size_t index, json_t *value);

/**
 * ringbuffer_append - Add value at end of ringbuffer
 *
 * @ringbuffer: The ringbuffer object
 * @value: The value
 *
 * The value appended is "stolen" in the sense that ringbuffer
 * doesn't increment its refcount but decreases  the refcount when
 * the value is no longer needed.
 *
 * Returns 0 on success, -1 on failure (out of range).
 */
int ringbuffer_append(ringbuffer_t *ringbuffer, json_t *value);

/**
 * ringbuffer_append_ringbuffer - Append another ringbuffer at end
 *
 * @ringbuffer: The ringbuffer object to add value to
 * @other: The other ring buffer to get value from
 *
 * The value appended are "stolen" in the sense that ringbuffer
 * doesn't increment its refcount but decreases  the refcount when
 * the value is no longer needed.
 *
 * Returns 0 on success, -1 on failure (out of range).
 */
int ringbuffer_append_ringbuffer(ringbuffer_t *ringbuffer, ringbuffer_t *other);

/**
 * ringbuffer_get - Get a value associated with a index
 *
 * @ringbuffer: The ringbuffer object
 * @index: The index
 *
 * Returns value if it is found, or NULL otherwise.
 */
json_t *ringbuffer_get(ringbuffer_t *ringbuffer, size_t index);

/**
 * ringbuffer_del - Remove a value from the ringbuffer
 *
 * @ringbuffer: The ringbuffer object
 * @index: The index
 *
 * Returns 0 on success, or -1 if the key was not found.
 */
int ringbuffer_del(ringbuffer_t *ringbuffer, size_t index);

/**
 * ringbuffer_clear - Clear ringbuffer
 *
 * @ringbuffer: The ringbuffer object
 *
 * Removes all items from the ringbuffer.
 */
void ringbuffer_clear(ringbuffer_t *ringbuffer);

#endif