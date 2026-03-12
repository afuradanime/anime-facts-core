#include <stdio.h>
#include "../include/anime_facts_api.h"
#include "../include/build_dll.h"

static const char* anime_type_to_string(enum anime_type type) {
    switch (type) {
        case TV: return "TV";
        case OVA: return "OVA";
        case MOVIE: return "Movie";
        case SPECIAL: return "Special";
        case ONA: return "ONA";
        case MUSIC: return "Music";
        default: return "Unknown";
    }
}

static const char* anime_status_to_string(enum anime_status status) {
    switch (status) {
        case FINISHED_AIRING: return "Finished Airing";
        case CURRENTLY_AIRING: return "Currently Airing";
        case NOT_YET_AIRED: return "Not Yet Aired";
        default: return "Unknown";
    }
}

static const char* season_to_string(meteorological_season s) {
    switch (s) {
        case SPRING: return "Spring";
        case SUMMER: return "Summer";
        case FALL:   return "Fall";
        case WINTER: return "Winter";
        default:     return "Unknown";
    }
}

static const char* tag_type_to_string(tag_type_t t) {
    switch (t) {
        case TAG_GENRE: return "Genre";
        case TAG_THEME: return "Theme";
        case TAG_DEMOGRAPHIC: return "Demographic";
        case TAG_EXPLICIT_GENRE: return "Explicit";
        default: return "Unknown";
    }
}

static void print_string_array(const char* label, const string_array_t* arr) {
    if (arr->count == 0) return;

    printf("%s:\n", label);
    for (size_t i = 0; i < arr->count; ++i) {
        printf("  - %s\n", arr->items[i]);
    }
}

API void print_anime(const anime_t* a) {
    if (!a) return;

    printf("==================================================\n");
    printf("[%u] %s\n", a->id, a->title);
    printf("==================================================\n");

    if (a->url)
        printf("URL: %s\n", a->url);

    printf("\n-- Basic Info ------------------------------------\n");
    printf("Type: %s\n", anime_type_to_string(a->type));
    printf("Status: %s\n", anime_status_to_string(a->status));
    printf("Episodes: %u\n", a->episodes);
    printf("Airing: %s\n", a->airing ? "Yes" : "No");

    if (a->source)
        printf("Source: %s\n", a->source);

    if (a->duration)
        printf("Duration: %s\n", a->duration);

    printf("\n-- Dates -----------------------------------------\n");
    if (a->start_date)
        printf("Start Date: %s\n", a->start_date);
    if (a->end_date)
        printf("End Date: %s\n", a->end_date);

    if (a->season.season != UNDEFINED)
        printf("Season: %s %hu\n",
               season_to_string(a->season.season),
               a->season.year);

    printf("\n-- Broadcast -------------------------------------\n");
    if (a->broadcast.day)
        printf("Day: %s\n", a->broadcast.day);
    if (a->broadcast.time)
        printf("Time: %s\n", a->broadcast.time);
    if (a->broadcast.timezone)
        printf("Timezone: %s\n", a->broadcast.timezone);

    printf("\n-- Media -----------------------------------------\n");
    if (a->image_url)
        printf("Image: %s\n", a->image_url);
    if (a->small_image_url)
        printf("Small Image: %s\n", a->small_image_url);
    if (a->large_image_url)
        printf("Large Image: %s\n", a->large_image_url);
    if (a->trailer_embed_url)
        printf("Trailer: %s\n", a->trailer_embed_url);

    printf("\n-- Titles ----------------------------------------\n");
    print_string_array("Synonyms", &a->synonyms);

    printf("\n-- Description ----------------------------------\n");
    printf("%s\n\n", a->description);

    printf("\n-- Producers -------------------------------------\n");
    for (size_t i = 0; i < a->producers.count; ++i) {
        producer_t* p = &a->producers.items[i];
        printf("- %s", p->name);
        if (p->url)  printf(" | %s", p->url);
        printf("\n");
    }

    printf("\n-- Licensors -------------------------------------\n");
    for (size_t i = 0; i < a->licensors.count; ++i) {
        licensor_t* l = &a->licensors.items[i];
        printf("- %s", l->name);
        if (l->url)  printf(" | %s", l->url);
        printf("\n");
    }

    printf("\n-- Studios ---------------------------------------\n");
    for (size_t i = 0; i < a->studios.count; ++i) {
        studio_t* s = &a->studios.items[i];
        printf("- %s", s->name);
        if (s->url) printf(" | %s", s->url);
        printf("\n");
    }

    printf("\n-- Tags ------------------------------------------\n");
    for (size_t i = 0; i < a->tags.count; ++i) {
        tag_t* t = &a->tags.items[i];
        printf("- [%s] %s",
               tag_type_to_string(t->type),
               t->name);
        if (t->url) printf(" | %s", t->url);
        printf("\n");
    }

    printf("==================================================\n");
}