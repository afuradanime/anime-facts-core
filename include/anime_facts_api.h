#ifndef ANIME_FACTS_API_H
#define ANIME_FACTS_API_H

#include "anime.h"
#include "dynamic_array.h"
#include "pageable.h"

extern char* season_names[5];

// dll interface

// Set up
__declspec(dllexport) void set_database_path(const char* new_path);
__declspec(dllexport) void close_db();

// Access database
__declspec(dllexport) int fetch_anime_by_id(unsigned int id, anime_t* data);

__declspec(dllexport) season_t current_season();
__declspec(dllexport) int fetch_anime_this_season(unsigned int* n, partial_anime_t** data);

__declspec(dllexport) int fetch_anime_from_query(const char* name, pageable_t page, unsigned int* n, partial_anime_t** data);

__declspec(dllexport) void free_anime(anime_t* data);
__declspec(dllexport) void free_anime_array(anime_t* data, unsigned int n);

__declspec(dllexport) void free_partial_anime(partial_anime_t* anime);
__declspec(dllexport) void free_partial_anime_array(partial_anime_t* data, unsigned int n);
#endif