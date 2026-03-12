#include "../include/anime.h"
#include <stdlib.h>
#include <string.h>

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
) {
    if (!anime) return;

    anime->id = id;
    anime->url = url ? strdup(url) : NULL;
    anime->title = title ? strdup(title) : NULL;
    
    anime->type = type;
    anime->status = status;
    
    anime->source = source ? strdup(source) : NULL;
    anime->episodes = episodes;
    
    anime->airing = airing;
    anime->duration = duration ? strdup(duration) : NULL;
    
    anime->start_date = start_date ? strdup(start_date) : NULL;
    anime->end_date = end_date ? strdup(end_date) : NULL;
    
    anime->season.season = season;
    anime->season.year = year;
    
    anime->broadcast.day = broadcast_day ? strdup(broadcast_day) : NULL;
    anime->broadcast.time = broadcast_time ? strdup(broadcast_time) : NULL;
    anime->broadcast.timezone = broadcast_timezone ? strdup(broadcast_timezone) : NULL;
    
    anime->image_url = image_url ? strdup(image_url) : NULL;
    anime->small_image_url = small_image_url ? strdup(small_image_url) : NULL;
    anime->large_image_url = large_image_url ? strdup(large_image_url) : NULL;
    
    anime->trailer_embed_url = trailer_embed_url ? strdup(trailer_embed_url) : NULL;
}

anime_t map_partial_anime(partial_anime_t* partial) {

    if (!partial) {
        anime_t empty = {0};
        return empty;
    }
    
    anime_t anime = {0};
    
    // Copy all partial fields
    anime.id = partial->id;
    anime.url = partial->url ? strdup(partial->url) : NULL;
    anime.title = partial->title ? strdup(partial->title) : NULL;
    anime.type = partial->type;
    anime.source = partial->source ? strdup(partial->source) : NULL;
    anime.episodes = partial->episodes;
    anime.status = partial->status;
    anime.airing = partial->airing;
    anime.duration = partial->duration ? strdup(partial->duration) : NULL;
    anime.start_date = partial->start_date ? strdup(partial->start_date) : NULL;
    anime.end_date = partial->end_date ? strdup(partial->end_date) : NULL;
    anime.season = partial->season;
    anime.broadcast.day = partial->broadcast.day ? strdup(partial->broadcast.day) : NULL;
    anime.broadcast.time = partial->broadcast.time ? strdup(partial->broadcast.time) : NULL;
    anime.broadcast.timezone = partial->broadcast.timezone ? strdup(partial->broadcast.timezone) : NULL;
    anime.image_url = partial->image_url ? strdup(partial->image_url) : NULL;
    anime.small_image_url = partial->small_image_url ? strdup(partial->small_image_url) : NULL;
    anime.large_image_url = partial->large_image_url ? strdup(partial->large_image_url) : NULL;
    anime.trailer_embed_url = partial->trailer_embed_url ? strdup(partial->trailer_embed_url) : NULL;
    
    init_string_array(&anime.synonyms);
    init_tag_array(&anime.tags);
    init_producer_array(&anime.producers);
    init_licensor_array(&anime.licensors);
    init_studio_array(&anime.studios);
    anime.description = NULL;
    
    return anime;
}

void add_anime_synonym(anime_t* anime, const char* synonym) {
    if (!anime || !synonym) return;
    push_string(&anime->synonyms, synonym);
}

void add_anime_producer(anime_t* anime, unsigned int id, const char* name, const char* url) {
    if (!anime || !name) return;
    push_producer(&anime->producers, id, name, url);
}

void add_anime_licensor(anime_t* anime, unsigned int id, const char* name, const char* url) {
    if (!anime || !name) return;
    push_licensor(&anime->licensors, id, name, url);
}

void add_anime_studio(anime_t* anime, unsigned int id, const char* name, const char* url) {
    if (!anime || !name) return;
    push_studio(&anime->studios, id, name, url);
}

void add_anime_tag(anime_t* anime, unsigned int id, const char* name, unsigned char tag_type, const char* url) {
    if (!anime || !name) return;
    push_tag(&anime->tags, id, name, tag_type, url);
}