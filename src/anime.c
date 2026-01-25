#include "../include/anime.h"

void make_anime_simple(
    anime_t* anime,
    unsigned int id, 
    const char *sources, 
    const char *title, 
    unsigned char type, 
    unsigned int episodes, 
    unsigned char status, 
    const char *picture, 
    const char *thumbnail
) {

    anime->id = id;

    anime->type = type;
    anime->episodes = episodes;
    anime->status = status;

    anime->picture = picture ? strdup(picture) : NULL;
    anime->thumbnail = thumbnail ? strdup(thumbnail) : NULL;

    // Always keeping in my mind that these strings may be null
    anime->sources = sources ? strdup(sources) : NULL;
    anime->title = title ? strdup(title) : NULL;

    // Initialize other data as null
    // We probably wont need all of this in full everytime, and it's taking a long time to fetch,
    // So we should maybe offer to query the "unimportant" info seperatly
    anime->descriptions[0] = NULL;
    anime->descriptions[1] = NULL;

    anime->duration_value = NULL;
    anime->season = (season_t) {0};

    init_string_array(&anime->synonyms);
    init_string_array(&anime->related_anime);

    init_tag_array(&anime->tags);
    init_producer_array(&anime->producers);
    init_studio_array(&anime->studios);
}