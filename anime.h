#ifndef ANIME_H
#define ANIME_H

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

typedef struct season {
    enum meteorological_season {
        SPRING, SUMMER, FALL, WINTER, UNDEFINED
    } season;
    unsigned short year;
} season_t;

typedef struct description {
    enum language {
        PORTUGUESE = 2, ENGLISH = 1
    } language;
    char* description;
} description_t;

typedef struct producer {
    char* producer;
} producer_t;

typedef struct studio {
    char* studio;
} studio_t;

typedef struct tag {
    char* tag;
} tag_t;

// Dynamic array helpers
typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} string_array_t;

typedef struct {
    tag_t* items;
    size_t count;
    size_t capacity;
} tag_array_t;

typedef struct {
    producer_t* items;
    size_t count;
    size_t capacity;
} producer_array_t;

typedef struct {
    studio_t* items;
    size_t count;
    size_t capacity;
} studio_array_t;

struct anime {
    unsigned int id;
    char* title;                                    // Main title
    string_array_t synonyms;                        // Other titles this anime goes by

    char* sources;                                  // URLs to the pages of the meta data providers for this anime
    description_t* descriptions[2];                 // Anime descriptions (Nullable)
    season_t season;                                // The season it aired

    enum anime_type {
        MOVIE, ONA, OVA, SPECIAL, TV, UNKNOWN_TYPE
    } type;                                         // Distribution type
    
    unsigned int episodes;                          // Number of episodes, movies or parts
    float* duration_value;                          // Duration per episode in seconds (or NULL)

    enum anime_status {
        FINISHED, ONGOING, UPCOMING, UNKNOWN_STATUS
    } status;                                       // Status of distribution
    
    char* picture;                                  // URL of a picture which represents the anime
    char* thumbnail;                                // URL of a smaller version of the picture
    
    string_array_t related_anime;                   // Related anime URLs
    tag_array_t tags;                               // Anime genres

    producer_array_t producers;                     // Producers that worked in the anime
    studio_array_t studios;                         // Studios that made the anime
};

typedef struct anime anime_t;

// Initialize dynamic arrays
static void init_string_array(string_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static void push_string(string_array_t* arr, const char* str) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (char**) realloc(arr->items, arr->capacity * sizeof(char*));
    }
    arr->items[arr->count++] = strdup(str);
}

static void init_tag_array(tag_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static void push_tag(tag_array_t* arr, const char* tag) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (tag_t*) realloc(arr->items, arr->capacity * sizeof(tag_t));
    }
    arr->items[arr->count++] = (tag_t){.tag = strdup(tag)};
}

static void init_producer_array(producer_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static void push_producer(producer_array_t* arr, const char* producer) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (producer_t*) realloc(arr->items, arr->capacity * sizeof(producer_t));
    }
    arr->items[arr->count++] = (producer_t){.producer = strdup(producer)};
}

static void init_studio_array(studio_array_t* arr) {
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static void push_studio(studio_array_t* arr, const char* studio) {
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        arr->items = (studio_t*) realloc(arr->items, arr->capacity * sizeof(studio_t));
    }
    arr->items[arr->count++] = (studio_t){.studio = strdup(studio)};
}

// Clean up
static void free_string_array(string_array_t* arr) {
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

static void free_tag_array(tag_array_t* arr) {
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

static void free_producer_array(producer_array_t* arr) {
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

static void free_studio_array(studio_array_t* arr) {
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

#endif