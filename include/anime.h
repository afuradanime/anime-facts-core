#ifndef ANIME_H
#define ANIME_H

#include "dynamic_array.h"
#include <string.h>
#include <stdbool.h>

typedef struct season {
    enum meteorological_season {
        SPRING, SUMMER, FALL, WINTER, UNDEFINED
    } season;
    unsigned short year;
} season_t;

typedef struct description {
    enum language {
        ENGLISH = 1, PORTUGUESE = 2
    } language;
    char* description;
} description_t;

typedef struct producer {
    unsigned int id;
    char* name;
    char* type;
    char* url;
} producer_t;

typedef struct licensor {
    unsigned int id;
    char* name;
    char* type;
    char* url;
} licensor_t;

typedef struct studio {
    unsigned int id;
    char* name;
    char* url;
} studio_t;

typedef struct tag {
    unsigned int id;
    char* name;
    enum tag_type {
        TAG_GENRE,
        TAG_THEME,
        TAG_DEMOGRAPHIC,
        TAG_EXPLICIT_GENRE
    } type;
    char* url;
} tag_t;

typedef struct broadcast {
    char* day;              // "Saturdays"
    char* time;             // "01:00"
    char* timezone;         // "Asia/Tokyo"
} broadcast_t;

enum anime_type {
    TV = 1, OVA, MOVIE, SPECIAL, ONA, MUSIC, UNKNOWN_TYPE
};

enum anime_status {
    FINISHED_AIRING = 1, CURRENTLY_AIRING, NOT_YET_AIRED, UNKNOWN_STATUS
} ;

/**
 * @brief The complete anime struct, with all information available
 */
struct anime {
    unsigned int id;
    char* url;              // MyAnimeList URL
    char* title;            // Main title
    
    string_array_t synonyms;                        // Title variants (Japanese, English, Synonyms)
    description_array_t descriptions;
    
    enum anime_type type;
    
    char* source;           // Original source ("Original", "Manga", "Light novel")
    unsigned int episodes;
    
    enum anime_status status;
    
    bool airing;            // Currently airing flag
    char* duration;         // Duration string "24 min per ep")
    
    // Dates
    char* start_date;       // ISO 8601 format
    char* end_date;         // ISO 8601 format (nullable)
    season_t season;        // Season aired
    
    broadcast_t broadcast;
    
    char* image_url;
    char* small_image_url;
    char* large_image_url;
    
    char* trailer_embed_url;
    
    tag_array_t tags;
    producer_array_t producers;
    licensor_array_t licensors;
    studio_array_t studios;
};

typedef struct anime anime_t;

/**
 * @brief A struct with partial anime information, used when searching 
 * large quantities of data to avoid time losses, they can be turned into 
 * complete anime manually later
 */
struct partial_anime {
    unsigned int id;
    char* url;              // MyAnimeList URL
    char* title;            // Main title
    
    enum anime_type type;
    
    char* source;           // Original source ("Original", "Manga", "Light novel")
    unsigned int episodes;
    
    enum anime_status status;
    
    bool airing;            // Currently airing flag
    char* duration;         // Duration string ("24 min per ep")
    
    // Dates
    char* start_date;       // ISO 8601 format
    char* end_date;         // ISO 8601 format (nullable)
    season_t season;        // Season aired
    
    broadcast_t broadcast;
    
    char* image_url;
    char* small_image_url;
    char* large_image_url;
    
    char* trailer_embed_url;
};

typedef struct partial_anime partial_anime_t;

// Filter by certain anime fields
// Everything is nullable
struct anime_filter {
    enum anime_type* type;
    enum anime_status* status;

    time_t* start_date;
    time_t* end_date;
    season_t* season;

    char* name;                     // Name search
    unsigned int* min_episodes;
    unsigned int* max_episodes;
};

typedef struct anime_filter anime_filter_t;

/**
 * @brief Create anime from database row data
 */
void make_partial_anime(
    partial_anime_t* anime,
    unsigned int id,
    const char* url,
    const char* title,
    unsigned char type,
    const char* source,
    unsigned int episodes,
    unsigned char status,
    bool airing,
    const char* duration,
    const char* start_date,
    const char* end_date,
    unsigned char season,
    unsigned short year,
    const char* broadcast_day,
    const char* broadcast_time,
    const char* broadcast_timezone,
    const char* image_url,
    const char* small_image_url,
    const char* large_image_url,
    const char* trailer_embed_url
);

/**
 * @brief Map a partial_anime_t to a full anime_t
 */
anime_t map_partial_anime(partial_anime_t* partial_anime);

/**
 * @brief Add a synonym (title variant) to anime
 */
void add_anime_synonym(anime_t* anime, const char* synonym);

/**
 * @brief Add a description in a specific language
 */
void add_anime_description(anime_t* anime, unsigned char language, const char* description);

/**
 * @brief Add a producer to anime
 */
void add_anime_producer(anime_t* anime, unsigned int id, const char* name, const char* type, const char* url);

/**
 * @brief Add a licensor to anime
 */
void add_anime_licensor(anime_t* anime, unsigned int id, const char* name, const char* type, const char* url);

/**
 * @brief Add a studio to anime
 */
void add_anime_studio(anime_t* anime, unsigned int id, const char* name, const char* url);

/**
 * @brief Add a tag (genre/theme/demographic) to anime
 */
void add_anime_tag(anime_t* anime, unsigned int id, const char* name, unsigned char tag_type, const char* url);

#endif