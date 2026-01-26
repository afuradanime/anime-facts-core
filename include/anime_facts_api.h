#ifndef ANIME_FACTS_API_H
#define ANIME_FACTS_API_H

#include "anime.h"
#include "dynamic_array.h"
#include "pageable.h"

extern char* season_names[5];

// dll interface
__declspec(dllexport) int fetch_anime_by_id(unsigned int id, anime_t* data);
__declspec(dllexport) season_t current_season();
__declspec(dllexport) int fetch_anime_this_season(unsigned int* n, anime_t** data);

__declspec(dllexport) void free_anime(anime_t* data);
__declspec(dllexport) void free_anime_array(anime_t* data, unsigned int n);
__declspec(dllexport) int fetch_anime_from_query(const char* name, pageable_t page, unsigned int* n, anime_t** data);
#endif