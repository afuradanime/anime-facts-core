#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <string.h>

typedef struct tag tag_t;
typedef struct producer producer_t;
typedef struct studio studio_t;

// Generic dynamic array
#define DEFINE_DYNAMIC_ARRAY(type_name, item_type) \
typedef struct { \
    item_type* items; \
    size_t count; \
    size_t capacity; \
} type_name;

// Dynamic arrays
DEFINE_DYNAMIC_ARRAY(string_array_t, char*)
DEFINE_DYNAMIC_ARRAY(tag_array_t, tag_t)
DEFINE_DYNAMIC_ARRAY(producer_array_t, producer_t)
DEFINE_DYNAMIC_ARRAY(studio_array_t, studio_t)

// Operations
void init_string_array(string_array_t* arr);
void push_string(string_array_t* arr, const char* str);
void free_string_array(string_array_t* arr);

void init_tag_array(tag_array_t* arr);
void push_tag(tag_array_t* arr, const char* tag);
void free_tag_array(tag_array_t* arr);

void init_producer_array(producer_array_t* arr);
void push_producer(producer_array_t* arr, const char* producer);
void free_producer_array(producer_array_t* arr);

void init_studio_array(studio_array_t* arr);
void push_studio(studio_array_t* arr, const char* studio);
void free_studio_array(studio_array_t* arr);

#endif