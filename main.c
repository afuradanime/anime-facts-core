#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sqlite3/sqlite3.h"

#include "anime_facts_api.h"

#define SQL(...) #__VA_ARGS__

#define DB_PATH "../anime.db"

char* season_names[5] =  {"SPRING", "SUMMER", "FALL", "WINTER", "UNDEFINED"};

__declspec(dllexport) void free_anime(anime_t data) {
    if (data.sources) free(data.sources);
    if (data.title) free(data.title);
    if (data.picture) free(data.picture);
    if (data.thumbnail) free(data.thumbnail);
    if (data.duration_value) free(data.duration_value);
}

__declspec(dllexport) void free_anime_array(anime_t* data, size_t n) {
    
    for (size_t i = 0; i < n; i++) free_anime(data[i]);
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

__declspec(dllexport) int fetch_anime_this_season(size_t* n, anime_t** data) {
    
    sqlite3* connection = NULL;
    *n = 0;
    *data = NULL;

    if (sqlite3_open(DB_PATH, &connection) != SQLITE_OK) {
        printf("Failed to open database\n");
        return 1;
    }
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.sources, a.title, a.type, a.episodes, a.status, a.picture, a.thumbnail, a.duration_value
        FROM anime a
        INNER JOIN anime_season a_season ON a.id = a_season.anime_id
        WHERE a_season.year = ? AND a_season.season = ?
        ORDER BY a.title;
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        printf("Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        sqlite3_close(connection);
        return 1;
    }

    season_t cur_season = current_season();

    int bind_rc1 = sqlite3_bind_int(stmt, 1, cur_season.year);
    if (bind_rc1 != SQLITE_OK) {
        printf("Failed to bind year rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_close(connection);
        return 1;
    }

    int bind_rc2 = sqlite3_bind_text(stmt, 2, season_names[cur_season.season], -1, NULL);
    if (bind_rc2 != SQLITE_OK) {
        printf("Failed to bind season rc:%d errMsg %s\n", bind_rc2, sqlite3_errmsg(connection));
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
        *data = (anime_t*) malloc(count * sizeof(anime_t));
        if (*data == NULL) {
            printf("Memory allocation failed\n");
            sqlite3_finalize(stmt);
            sqlite3_close(connection);
            return 1;
        }
    }

    // populate array
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {

        (*data)[i].id = sqlite3_column_int(stmt, 0);
        
        const char* sources = (const char*) sqlite3_column_text(stmt, 1);
        (*data)[i].sources = sources ? strdup(sources) : NULL;
        
        const char* title = (const char*) sqlite3_column_text(stmt, 2);
        (*data)[i].title = title ? strdup(title) : NULL;
        
        (*data)[i].type = sqlite3_column_int(stmt, 3);
        (*data)[i].episodes = sqlite3_column_int(stmt, 4);
        (*data)[i].status = sqlite3_column_int(stmt, 5);
        
        const char* picture = (const char*) sqlite3_column_text(stmt, 6);
        (*data)[i].picture = picture ? strdup(picture) : NULL;
        
        const char* thumbnail = (const char*) sqlite3_column_text(stmt, 7);
        (*data)[i].thumbnail = thumbnail ? strdup(thumbnail) : NULL;
        
        (*data)[i].duration_value = NULL;
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
            (*data)[i].duration_value = (float*) malloc(sizeof(float));
            *(*data)[i].duration_value = sqlite3_column_double(stmt, 8);
        }
        
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
    size_t n = 0;
    
    if (fetch_anime_this_season(&n, &data) == 0 && n > 0) {

        for(size_t i = 0; i < n; i++)
            printf("[%d] %s\n", data[i].id, data[i].title);

        free_anime_array(data, n);
    }

    return 0;
}