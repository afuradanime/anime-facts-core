#include "../include/dynamic_array.h"
#include "../include/anime.h"
#include <stdlib.h>
#include <string.h>

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

void init_tag_array(tag_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_tag(tag_array_t* arr, unsigned int id, const char* name, unsigned char type, const char* url) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (tag_t*) realloc(arr->items, arr->capacity * sizeof(tag_t));
    }
    arr->items[arr->count++] = (tag_t){
        .id = id,
        .name = strdup(name),
        .type = type,
        .url = url ? strdup(url) : NULL
    };
}

void free_tag_array(tag_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].name) free(arr->items[i].name);
            if (arr->items[i].url) free(arr->items[i].url);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void init_producer_array(producer_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_producer(producer_array_t* arr, unsigned int id, const char* name, const char* url) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (producer_t*) realloc(arr->items, arr->capacity * sizeof(producer_t));
    }
    arr->items[arr->count++] = (producer_t){
        .id = id,
        .name = strdup(name),
        .url = url ? strdup(url) : NULL
    };
}

void free_producer_array(producer_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].name) free(arr->items[i].name);
            if (arr->items[i].url) free(arr->items[i].url);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void init_licensor_array(licensor_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_licensor(licensor_array_t* arr, unsigned int id, const char* name, const char* url) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (licensor_t*) realloc(arr->items, arr->capacity * sizeof(licensor_t));
    }
    arr->items[arr->count++] = (licensor_t){
        .id = id,
        .name = strdup(name),
        .url = url ? strdup(url) : NULL
    };
}

void free_licensor_array(licensor_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].name) free(arr->items[i].name);
            if (arr->items[i].url) free(arr->items[i].url);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void init_studio_array(studio_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void push_studio(studio_array_t* arr, unsigned int id, const char* name, const char* url) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (studio_t*) realloc(arr->items, arr->capacity * sizeof(studio_t));
    }
    arr->items[arr->count++] = (studio_t){
        .id = id,
        .name = strdup(name),
        .url = url ? strdup(url) : NULL
    };
}

void free_studio_array(studio_array_t* arr) {
    if (arr->items) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->items[i].name) free(arr->items[i].name);
            if (arr->items[i].url) free(arr->items[i].url);
        }
        free(arr->items);
    }
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}