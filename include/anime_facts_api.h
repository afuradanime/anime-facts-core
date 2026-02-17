#ifndef ANIME_FACTS_API_H
#define ANIME_FACTS_API_H

#include "build_dll.h"
#include "anime.h"
#include "dynamic_array.h"
#include "pageable.h"

extern char* season_names[5];

// dll interface

// Set up
API void set_database_path(const char* new_path);
API void close_db();

// Anime interface
API int fetch_anime_by_id(unsigned int id, anime_t* data);
API int fetch_anime_this_season(unsigned int* n, partial_anime_t** data);
API int fetch_anime_from_query(const char* name, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** data);
API int fetch_anime_from_tag(unsigned int tag_id, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** data);

API void free_anime(anime_t* data);
API void free_anime_array(anime_t* data, unsigned int n);

API void free_partial_anime(partial_anime_t* anime);
API void free_partial_anime_array(partial_anime_t* data, unsigned int n);

// Entity interface
API int fetch_studio_by_id(unsigned int id, studio_t* data, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** anime);
API int fetch_producer_by_id(unsigned int id, producer_t* data, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** anime);
API int fetch_licensor_by_id(unsigned int id, licensor_t* data, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** anime);

API void free_studio(studio_t* data);
API void free_producer(producer_t* data);
API void free_licensor(licensor_t* data);

// Utilities
API season_t current_season();
API void print_anime(const anime_t* a);
#endif