#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct tag tag_t;
typedef struct producer producer_t;
typedef struct licensor licensor_t;
typedef struct studio studio_t;
typedef struct description description_t;

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
DEFINE_DYNAMIC_ARRAY(licensor_array_t, licensor_t)
DEFINE_DYNAMIC_ARRAY(studio_array_t, studio_t)
DEFINE_DYNAMIC_ARRAY(description_array_t, description_t)

// String array operations
void init_string_array(string_array_t* arr);
void push_string(string_array_t* arr, const char* str);
void free_string_array(string_array_t* arr);

// Tag array operations
void init_tag_array(tag_array_t* arr);
void push_tag(tag_array_t* arr, unsigned int id, const char* name, unsigned char type, const char* url);
void free_tag_array(tag_array_t* arr);

// Producer array operations
void init_producer_array(producer_array_t* arr);
void push_producer(producer_array_t* arr, unsigned int id, const char* name, const char* type, const char* url);
void free_producer_array(producer_array_t* arr);

// Licensor array operations
void init_licensor_array(licensor_array_t* arr);
void push_licensor(licensor_array_t* arr, unsigned int id, const char* name, const char* type, const char* url);
void free_licensor_array(licensor_array_t* arr);

// Studio array operations
void init_studio_array(studio_array_t* arr);
void push_studio(studio_array_t* arr, unsigned int id, const char* name, const char* url);
void free_studio_array(studio_array_t* arr);

// Description array operations
void init_description_array(description_array_t* arr);
void push_description(description_array_t* arr, unsigned char language, const char* description);
void free_description_array(description_array_t* arr);

#endif