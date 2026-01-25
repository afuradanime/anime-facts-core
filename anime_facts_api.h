#ifndef ANIME_FACTS_API_H
#define ANIME_FACTS_API_H

extern char* season_names[5];
struct season {
    enum meteorological_season {
        SPRING, SUMMER, FALL, WINTER, UNDEFINED
    } season;
    unsigned short year;
};

typedef struct season season_t;

struct description {
    enum language {
        PORTUGUESE, ENGLISH
    } language;
    char* description;
};

typedef struct description description_t;

struct producer {
    char* producer;
};

typedef struct producer producer_t;

struct studio {
    char* studio;
};

typedef struct studio studio_t;

struct tag {
    char* tag;
};

typedef struct tag tag_t;

struct anime {
    unsigned int id;
    char* sources; // URLs to the pages of the meta data providers for this anime
    char* title; // Main title
    description_t* descriptions[2]; // Anime descriptions (Nullable)

    enum anime_type {
        MOVIE, ONA, OVA, SPECIAL, TV, UNKNOWN_TYPE
    } type; // Distribution type
    
    unsigned int episodes; // Number of episodes, movies or parts
    float* duration_value; // Duration per episode in seconds (or NULL)

    enum anime_status {
        FINISHED, ONGOING, UPCOMING, UNKNOWN_STATUS
    } status; // Status of distribution
    
    char* picture; // URL of a picture which represents the anime
    char* thumbnail; // URL of a smaller version of the picture
    
    char** related_anime; // Related anime URLs
    tag_t* tags; // Anime genres
};

typedef struct anime anime_t;

// dll interface
__declspec(dllexport) season_t current_season();
__declspec(dllexport) int fetch_anime_this_season(size_t* n, anime_t** data);

__declspec(dllexport) void free_anime(anime_t data);
__declspec(dllexport) void free_anime_array(anime_t* data, size_t n);

#endif