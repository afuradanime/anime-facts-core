#include "../include/dynamic_array.h"
#include "../include/anime.h"

// Initialize dynamic arrays
void init_string_array(string_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_string(string_array_t* arr, const char* str) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (char**) realloc(arr->items, arr->capacity * sizeof(char*));
    }
    arr->items[arr->count++] = strdup(str);
}

void init_tag_array(tag_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_tag(tag_array_t* arr, const char* tag) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (tag_t*) realloc(arr->items, arr->capacity * sizeof(tag_t));
    }
    arr->items[arr->count++] = (tag_t){.tag = strdup(tag)};
}

void init_producer_array(producer_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_producer(producer_array_t* arr, const char* producer) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (producer_t*) realloc(arr->items, arr->capacity * sizeof(producer_t));
    }
    arr->items[arr->count++] = (producer_t){.producer = strdup(producer)};
}

void init_studio_array(studio_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_studio(studio_array_t* arr, const char* studio) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (studio_t*) realloc(arr->items, arr->capacity * sizeof(studio_t));
    }
    arr->items[arr->count++] = (studio_t){.studio = strdup(studio)};
}

// Clean up
void free_string_array(string_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            free(arr->items[i]);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void free_tag_array(tag_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].tag) free(arr->items[i].tag);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void free_producer_array(producer_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].producer) free(arr->items[i].producer);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void free_studio_array(studio_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].studio) free(arr->items[i].studio);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}