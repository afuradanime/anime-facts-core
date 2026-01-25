#ifndef ANIME_FACTS_API_H
#define ANIME_FACTS_API_H

#include "anime.h"
#include "dynamic_array.h"

extern char* season_names[5];

// dll interface
__declspec(dllexport) int fetch_anime_by_id(unsigned int id, anime_t* data);
__declspec(dllexport) season_t current_season();
__declspec(dllexport) int fetch_anime_this_season(size_t* n, anime_t** data);

__declspec(dllexport) void free_anime(anime_t* data);
__declspec(dllexport) void free_anime_array(anime_t* data, size_t n);

#endif