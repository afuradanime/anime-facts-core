#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "include/anime.h"
#include "include/pageable.h"
#include "include/anime_facts_api.h"
#include "include/dynamic_array.h"

#include "sqlite3/sqlite3.h"

#define SQL(...) #__VA_ARGS__

#define DB_PATH "../anime.db"

char* season_names[5] =  {"SPRING", "SUMMER", "FALL", "WINTER", "UNDEFINED"};

__declspec(dllexport) void free_anime(anime_t* anime) {
    if (!anime) return;
    
    if (anime->sources) free(anime->sources);
    if (anime->title) free(anime->title);
    if (anime->picture) free(anime->picture);
    if (anime->thumbnail) free(anime->thumbnail);
    if (anime->duration_value) free(anime->duration_value);
    
    // Free descriptions
    for (int i = 0; i < 2; i++) {
        if (anime->descriptions[i]) {
            if (anime->descriptions[i]->description) {
                free(anime->descriptions[i]->description);
            }
            free(anime->descriptions[i]);
        }
    }
    
    // Free dynamic arrays
    free_string_array(&anime->synonyms);
    free_string_array(&anime->related_anime);
    free_tag_array(&anime->tags);
    free_producer_array(&anime->producers);
    free_studio_array(&anime->studios);
}

__declspec(dllexport) void free_anime_array(anime_t* data, unsigned int n) {
    
    for (size_t i = 0; i < n; i++) free_anime(&data[i]);
    free(data);
}

__declspec(dllexport) season_t current_season() {

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    int year = t->tm_year + 1900; // Current year
    int day = t->tm_mday; // Current day
    int month = t->tm_mon + 1; // Current month

    enum meteorological_season season;
    
    // Spring: March 20 - June 20
    // Summer: June 21 - September 22
    // Fall: September 23 - December 21
    // Winter: December 22 - March 19
    if ((month == 3 && day >= 20) || (month > 3 && month < 6) || (month == 6 && day < 21)) season = SPRING;
    else if ((month == 6 && day >= 21) || (month > 6 && month < 9) || (month == 9 && day < 23)) season = SUMMER;
    else if ((month == 9 && day >= 23) || (month > 9 && month < 12) || (month == 12 && day < 22)) season = FALL;
    else season = WINTER;

    return (season_t) {
        .season = season,
        .year = (unsigned short) year
    };
}

__declspec(dllexport) int fetch_anime_from_query(const char* name, pageable_t page, unsigned int* n, anime_t** data) {

    sqlite3* connection = NULL;
    *n = 0;
    *data = NULL;

    if (sqlite3_open(DB_PATH, &connection) != SQLITE_OK) {
        fprintf(stderr, "Failed to open database\n");
        return 1;
    }
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.sources, a.title, a.type, a.episodes, a.status, a.picture, a.thumbnail, a.duration_value
        FROM anime a
        WHERE a.title LIKE ?
        ORDER BY a.title
        LIMIT ? OFFSET ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        sqlite3_close(connection);
        return 1;
    }

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%%%s%%", name);

    int bind_rc1 = sqlite3_bind_text(stmt, 1, pattern, -1, NULL);
    if (bind_rc1 != SQLITE_OK) {
        fprintf(stderr, "Failed to bind name filter rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    int bind_rc2 = sqlite3_bind_int(stmt, 2, page.page_size);
    if (bind_rc2 != SQLITE_OK) {
        fprintf(stderr, "Failed to bind page size filter rc:%d errMsg %s\n", bind_rc2, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    int bind_rc3 = sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);
    if (bind_rc3 != SQLITE_OK) {
        fprintf(stderr, "Failed to bind page number filter rc:%d errMsg %s\n", bind_rc3, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, pattern, -1, NULL);
    sqlite3_bind_int(stmt, 2, page.page_size);
    sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);

    // Allocate array
    if (count > 0) {
        *data = (anime_t*) calloc(count, sizeof(anime_t));
        if (*data == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            sqlite3_finalize(stmt);
            sqlite3_close(connection);
            return 1;
        }
    }

    // populate array
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {

        anime_t* anime = &(*data)[i];
        
        make_anime_simple(
            anime, 
            sqlite3_column_int(stmt, 0), 
            (const char*) sqlite3_column_text(stmt, 1), 
            (const char*) sqlite3_column_text(stmt, 2), 
            sqlite3_column_int(stmt, 3), 
            sqlite3_column_int(stmt, 4), 
            sqlite3_column_int(stmt, 5), 
            (const char*) sqlite3_column_text(stmt, 6), 
            (const char*) sqlite3_column_text(stmt, 7)
        );
        
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) 
            set_anime_duration(anime, sqlite3_column_double(stmt, 8));
        
        i++;
    }

    *n = count;

    sqlite3_finalize(stmt);
    sqlite3_close(connection);

    return 0;
}

__declspec(dllexport) int fetch_anime_by_id(unsigned int id, anime_t* data) {
    
    if (!data) return 1;

    sqlite3* connection = NULL;

    if (sqlite3_open(DB_PATH, &connection) != SQLITE_OK) {
        fprintf(stderr, "Failed to open database\n");
        return 1;
    }

    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.sources, a.title, a.type, a.episodes, a.status, a.picture, a.thumbnail, a.duration_value
        FROM anime a
        WHERE a.id = ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        sqlite3_close(connection);
        return 1;
    }

    int bind_rc1 = sqlite3_bind_int(stmt, 1, id);
    if (bind_rc1 != SQLITE_OK) {
        fprintf(stderr, "Failed to bind id rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    int step_rc = sqlite3_step(stmt);
    if (step_rc != SQLITE_ROW) {
        fprintf(stderr, "No anime found with id %u\n", id);
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;  // Not found / error
    }

    make_anime_simple(
        data, 
        sqlite3_column_int(stmt, 0), 
        (const char*) sqlite3_column_text(stmt, 1), 
        (const char*) sqlite3_column_text(stmt, 2), 
        sqlite3_column_int(stmt, 3), 
        sqlite3_column_int(stmt, 4), 
        sqlite3_column_int(stmt, 5), 
        (const char*) sqlite3_column_text(stmt, 6), 
        (const char*) sqlite3_column_text(stmt, 7)
    );
    
    if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) 
        set_anime_duration(data, sqlite3_column_double(stmt, 8));

    sqlite3_finalize(stmt);
    sqlite3_close(connection);

    return 0;
}

__declspec(dllexport) int fetch_anime_this_season(unsigned int* n, anime_t** data) {
    
    sqlite3* connection = NULL;
    *n = 0;
    *data = NULL;

    if (sqlite3_open(DB_PATH, &connection) != SQLITE_OK) {
        fprintf(stderr, "Failed to open database\n");
        return 1;
    }
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.sources, a.title, a.type, a.episodes, a.status, a.picture, a.thumbnail, a.duration_value, s.year, s.season
        FROM anime a
        INNER JOIN anime_season s ON a.id = s.anime_id
        WHERE s.year = ? AND s.season = ?
        ORDER BY a.title
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        sqlite3_close(connection);
        return 1;
    }

    season_t cur_season = current_season();

    int bind_rc1 = sqlite3_bind_int(stmt, 1, cur_season.year);
    if (bind_rc1 != SQLITE_OK) {
        fprintf(stderr, "Failed to bind year rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    int bind_rc2 = sqlite3_bind_text(stmt, 2, season_names[cur_season.season], -1, NULL);
    if (bind_rc2 != SQLITE_OK) {
        fprintf(stderr, "Failed to bind season rc:%d errMsg %s\n", bind_rc2, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, cur_season.year);
    sqlite3_bind_text(stmt, 2, season_names[cur_season.season], -1, NULL);

    // Allocate array
    if (count > 0) {
        *data = (anime_t*) calloc(count, sizeof(anime_t));
        if (*data == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            sqlite3_finalize(stmt);
            sqlite3_close(connection);
            return 1;
        }
    }

    // populate array
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {

        anime_t* anime = &(*data)[i];
        
        make_anime_simple(
            anime, 
            sqlite3_column_int(stmt, 0), 
            (const char*) sqlite3_column_text(stmt, 1), 
            (const char*) sqlite3_column_text(stmt, 2), 
            sqlite3_column_int(stmt, 3), 
            sqlite3_column_int(stmt, 4), 
            sqlite3_column_int(stmt, 5), 
            (const char*) sqlite3_column_text(stmt, 6), 
            (const char*) sqlite3_column_text(stmt, 7)
        );
        
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) 
            set_anime_duration(anime, sqlite3_column_double(stmt, 8));
        
        set_anime_season(
            anime,
            sqlite3_column_int(stmt, 9),
            sqlite3_column_int(stmt, 10)
        );
        
        i++;
    }

    *n = count;

    sqlite3_finalize(stmt);
    sqlite3_close(connection);

    return 0;
}

int main(void) {

    printf("Hello, world!\n\n");

    anime_t* data = NULL;
    unsigned int n = 0;
    
    if (fetch_anime_from_query("prisma", (pageable_t) {
        .page_number = 0,
        .page_size = 10
    }, &n, &data) == 0 && n > 0) {

        for(size_t i = 0; i < n; i++)
            printf("[%d] %s\n", data[i].id, data[i].title);

        free_anime_array(data, n);
    }

    // anime_t a;
    // if (fetch_anime_by_id(25032, &a) == 0) {

    //     printf("[%d] %s\n", a.id, a.title);

    //     free_anime(&a);
    // }
    return 0;
}