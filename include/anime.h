#ifndef ANIME_H
#define ANIME_H

#include "dynamic_array.h"

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

#endif