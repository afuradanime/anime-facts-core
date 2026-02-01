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

// Path to the database file
#define MAX_PATH_LEN 1024
static char DB_PATH[MAX_PATH_LEN] = "anime.db";

char* season_names[5] =  {"spring", "summer", "fall", "winter", "unknown"};

// log_msg function to make it quiet when benchmarking
static void log_msg(FILE* s, const char* c, ...) {
    #ifndef QUIET
    va_list args;
    va_start(args, c);
    vfprintf(s, c, args);
    va_end(args);
    #endif
}

// A global sqlite connection
/**
 * We have a global connection to avoid the overhead of opening and closing
 * the database file every query, also allows us to keep sqlite cache
*/ 
static sqlite3* db_conn = NULL;

static sqlite3* get_db() {

    if (db_conn) return db_conn;

    if (sqlite3_open(DB_PATH, &db_conn) != SQLITE_OK) {
        log_msg(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db_conn));
        sqlite3_close(db_conn);
        db_conn = NULL;
        return NULL;
    }

    return db_conn;
}

// Manual closing shouldn't be necessary, but if you want, for whatever reason, to unlock
// the database file, there you go
// TODO: Currently necessary on non windows systems!
__declspec(dllexport) void close_db() {
    if (db_conn) {
        sqlite3_close(db_conn);
        db_conn = NULL;
    }
}

__declspec(dllexport) void set_database_path(const char* new_path) {

    strncpy(DB_PATH, new_path, MAX_PATH_LEN - 1);
    DB_PATH[MAX_PATH_LEN - 1] = '\0';
}

__declspec(dllexport) void free_anime(anime_t* anime) {
    if (!anime) return;
    
    // Free strings
    if (anime->url) free(anime->url);
    if (anime->title) free(anime->title);
    if (anime->source) free(anime->source);
    if (anime->duration) free(anime->duration);
    if (anime->start_date) free(anime->start_date);
    if (anime->end_date) free(anime->end_date);
    if (anime->broadcast.day) free(anime->broadcast.day);
    if (anime->broadcast.time) free(anime->broadcast.time);
    if (anime->broadcast.timezone) free(anime->broadcast.timezone);
    if (anime->image_url) free(anime->image_url);
    if (anime->small_image_url) free(anime->small_image_url);
    if (anime->large_image_url) free(anime->large_image_url);
    if (anime->trailer_embed_url) free(anime->trailer_embed_url);
    
    // Free arrays
    free_string_array(&anime->synonyms);
    free_description_array(&anime->descriptions);
    free_tag_array(&anime->tags);
    free_producer_array(&anime->producers);
    free_licensor_array(&anime->licensors);
    free_studio_array(&anime->studios);
    
    memset(anime, 0, sizeof(anime_t));
}

__declspec(dllexport) void free_anime_array(anime_t* data, unsigned int n) {
    
    for (size_t i = 0; i < n; i++) free_anime(&data[i]);
    free(data);
}

__declspec(dllexport) void free_partial_anime(partial_anime_t* anime) {
    if (!anime) return;
    
    // Free strings
    if (anime->url) free(anime->url);
    if (anime->title) free(anime->title);
    if (anime->source) free(anime->source);
    if (anime->duration) free(anime->duration);
    if (anime->start_date) free(anime->start_date);
    if (anime->end_date) free(anime->end_date);
    if (anime->broadcast.day) free(anime->broadcast.day);
    if (anime->broadcast.time) free(anime->broadcast.time);
    if (anime->broadcast.timezone) free(anime->broadcast.timezone);
    if (anime->image_url) free(anime->image_url);
    if (anime->small_image_url) free(anime->small_image_url);
    if (anime->large_image_url) free(anime->large_image_url);
    if (anime->trailer_embed_url) free(anime->trailer_embed_url);
    
    memset(anime, 0, sizeof(partial_anime_t));
}

__declspec(dllexport) void free_partial_anime_array(partial_anime_t* data, unsigned int n) {
    
    for (size_t i = 0; i < n; i++) free_partial_anime(&data[i]);
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

static const char* get_text_or_null(sqlite3_stmt* stmt, int col) {
    return sqlite3_column_type(stmt, col) == SQLITE_NULL ? NULL : (const char*) sqlite3_column_text(stmt, col);
}

static unsigned char map_season_string(const char* season_str) {
    if (!season_str) return UNDEFINED;
    if (strcmp(season_str, "spring") == 0) return SPRING;
    if (strcmp(season_str, "summer") == 0) return SUMMER;
    if (strcmp(season_str, "fall") == 0) return FALL;
    if (strcmp(season_str, "winter") == 0) return WINTER;
    return UNDEFINED;
}

static unsigned char map_tag_type(const char* type) {
    if (!type) return TAG_GENRE;
    if (strcmp(type, "genre") == 0) return TAG_GENRE;
    if (strcmp(type, "theme") == 0) return TAG_THEME;
    if (strcmp(type, "demographic") == 0) return TAG_DEMOGRAPHIC;
    return TAG_EXPLICIT_GENRE;
}

__declspec(dllexport) int fetch_anime_from_query(const char* name, pageable_t page, unsigned int* n, partial_anime_t** data) {

    sqlite3* connection = get_db();
    if (!connection) return 1;

    *n = 0;
    *data = NULL;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime a
        WHERE a.title LIKE ?
        ORDER BY a.quality_score DESC, a.title ASC
        LIMIT ? OFFSET ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%%%s%%", name);

    int bind_rc1 = sqlite3_bind_text(stmt, 1, pattern, -1, NULL);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind name filter rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int bind_rc2 = sqlite3_bind_int(stmt, 2, page.page_size);
    if (bind_rc2 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind page size filter rc:%d errMsg %s\n", bind_rc2, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int bind_rc3 = sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);
    if (bind_rc3 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind page number filter rc:%d errMsg %s\n", bind_rc3, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
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
        *data = (partial_anime_t*) calloc(count, sizeof(partial_anime_t));
        if (*data == NULL) {
            log_msg(stderr, "Memory allocation failed\n");
            sqlite3_finalize(stmt);
            return 1;
        }
    }

    // populate array
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {

        partial_anime_t* anime = &(*data)[i];
        
        const char* season_str = get_text_or_null(stmt, 11);

        make_partial_anime(
            anime,
            sqlite3_column_int(stmt, 0),
            get_text_or_null(stmt, 1),
            get_text_or_null(stmt, 2),
            (unsigned char)sqlite3_column_int(stmt, 3),
            get_text_or_null(stmt, 4),
            sqlite3_column_int(stmt, 5),
            (unsigned char)sqlite3_column_int(stmt, 6),
            sqlite3_column_int(stmt, 7) != 0,
            get_text_or_null(stmt, 8),
            get_text_or_null(stmt, 9),
            get_text_or_null(stmt, 10),
            map_season_string(season_str),
            (unsigned short)sqlite3_column_int(stmt, 12),
            get_text_or_null(stmt, 13),
            get_text_or_null(stmt, 14),
            get_text_or_null(stmt, 15),
            get_text_or_null(stmt, 16),
            get_text_or_null(stmt, 17),
            get_text_or_null(stmt, 18),
            get_text_or_null(stmt, 19)
        );
        
        i++;
    }

    *n = count;

    sqlite3_finalize(stmt);
    return 0;
}

static int fetch_synonyms(sqlite3* db, unsigned int anime_id, anime_t* anime) {
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL(
        SELECT title FROM synonyms WHERE anime_id = ?
    ), -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_int(stmt, 1, anime_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* title = get_text_or_null(stmt, 0);
        if (title)
            add_anime_synonym(anime, title);
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int fetch_descriptions(sqlite3* db, unsigned int anime_id, anime_t* anime) {
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL(
        SELECT language_id, description
        FROM anime_descriptions
        WHERE anime_id = ?
    ), -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_int(stmt, 1, anime_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        unsigned char language = (unsigned char)sqlite3_column_int(stmt, 0);
        const char* desc = get_text_or_null(stmt, 1);

        if (desc)
            add_anime_description(anime, language, desc);
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int fetch_producers(sqlite3* db, unsigned int anime_id, anime_t* anime) {
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL(
        SELECT p.id, p.name, p.type, p.url
        FROM anime_producers ap
        JOIN producers p ON p.id = ap.producer_id
        WHERE ap.anime_id = ?
    ), -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_int(stmt, 1, anime_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        add_anime_producer(
            anime,
            sqlite3_column_int(stmt, 0),
            get_text_or_null(stmt, 1),
            get_text_or_null(stmt, 2),
            get_text_or_null(stmt, 3)
        );
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int fetch_licensors(sqlite3* db, unsigned int anime_id, anime_t* anime) {
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL(
        SELECT l.id, l.name, l.type, l.url
        FROM anime_licensors al
        JOIN licensors l ON l.id = al.licensor_id
        WHERE al.anime_id = ?
    ), -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_int(stmt, 1, anime_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        add_anime_licensor(
            anime,
            sqlite3_column_int(stmt, 0),
            get_text_or_null(stmt, 1),
            get_text_or_null(stmt, 2),
            get_text_or_null(stmt, 3)
        );
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int fetch_studios(sqlite3* db, unsigned int anime_id, anime_t* anime) {
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL(
        SELECT s.id, s.name, s.url
        FROM anime_studios _as
        JOIN studios s ON s.id = _as.studio_id
        WHERE _as.anime_id = ?
    ), -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_int(stmt, 1, anime_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        add_anime_studio(
            anime,
            sqlite3_column_int(stmt, 0),
            get_text_or_null(stmt, 1),
            get_text_or_null(stmt, 2)
        );
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int fetch_tags(sqlite3* db, unsigned int anime_id, anime_t* anime) {
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL(
        SELECT t.id, t.name, t.type, t.url
        FROM anime_tags at
        JOIN tags t ON t.id = at.tag_id
        WHERE at.anime_id = ?
    ), -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_int(stmt, 1, anime_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        add_anime_tag(
            anime,
            sqlite3_column_int(stmt, 0),
            get_text_or_null(stmt, 1),
            map_tag_type(get_text_or_null(stmt, 2)),
            get_text_or_null(stmt, 3)
        );
    }

    sqlite3_finalize(stmt);
    return 0;
}

__declspec(dllexport) int fetch_anime_by_id(unsigned int id, anime_t* data) {
    
    if (!data) return 1;

    sqlite3* connection = get_db();
    if (!connection) return 1;

    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime a
        WHERE a.id = ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    int bind_rc1 = sqlite3_bind_int(stmt, 1, id);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind id rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int step_rc = sqlite3_step(stmt);
    if (step_rc != SQLITE_ROW) {
        log_msg(stderr, "No anime found with id %u\n", id);
        sqlite3_finalize(stmt);
        return 1;  // Not found / error
    }

    const char* season_str = get_text_or_null(stmt, 11);

    partial_anime_t temp = {0};

    make_partial_anime(
        &temp,
        sqlite3_column_int(stmt, 0),
        get_text_or_null(stmt, 1),
        get_text_or_null(stmt, 2),
        (unsigned char)sqlite3_column_int(stmt, 3),
        get_text_or_null(stmt, 4),
        sqlite3_column_int(stmt, 5),
        (unsigned char)sqlite3_column_int(stmt, 6),
        sqlite3_column_int(stmt, 7) != 0,
        get_text_or_null(stmt, 8),
        get_text_or_null(stmt, 9),
        get_text_or_null(stmt, 10),
        map_season_string(season_str),
        (unsigned short)sqlite3_column_int(stmt, 12),
        get_text_or_null(stmt, 13),
        get_text_or_null(stmt, 14),
        get_text_or_null(stmt, 15),
        get_text_or_null(stmt, 16),
        get_text_or_null(stmt, 17),
        get_text_or_null(stmt, 18),
        get_text_or_null(stmt, 19)
    );

    *data = map_partial_anime(&temp);

    fetch_synonyms(connection, id, data);
    fetch_descriptions(connection, id, data);
    fetch_producers(connection, id, data);
    fetch_licensors(connection, id, data);
    fetch_studios(connection, id, data);
    fetch_tags(connection, id, data);

    sqlite3_finalize(stmt);
    return 0;
}

__declspec(dllexport) int fetch_anime_this_season(unsigned int* n, partial_anime_t** data) {
    
    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    *n = 0;
    *data = NULL;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime a
        WHERE a.year = ? AND a.season = ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    season_t cur_season = current_season();

    int bind_rc1 = sqlite3_bind_int(stmt, 1, cur_season.year);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind year rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int bind_rc2 = sqlite3_bind_text(stmt, 2, season_names[cur_season.season], -1, NULL);
    if (bind_rc2 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind season rc:%d errMsg %s\n", bind_rc2, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
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
        *data = (partial_anime_t*) calloc(count, sizeof(partial_anime_t));
        if (*data == NULL) {
            log_msg(stderr, "Memory allocation failed\n");
            sqlite3_finalize(stmt);
            return 1;
        }
    }

    // Populate array
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < count) {
        partial_anime_t* anime = &(*data)[i];
        
        const char* season_str = get_text_or_null(stmt, 11);

        make_partial_anime(
            anime,
            sqlite3_column_int(stmt, 0),
            get_text_or_null(stmt, 1),
            get_text_or_null(stmt, 2),
            (unsigned char)sqlite3_column_int(stmt, 3),
            get_text_or_null(stmt, 4),
            sqlite3_column_int(stmt, 5),
            (unsigned char)sqlite3_column_int(stmt, 6),
            sqlite3_column_int(stmt, 7) != 0,
            get_text_or_null(stmt, 8),
            get_text_or_null(stmt, 9),
            get_text_or_null(stmt, 10),
            map_season_string(season_str),
            (unsigned short)sqlite3_column_int(stmt, 12),
            get_text_or_null(stmt, 13),
            get_text_or_null(stmt, 14),
            get_text_or_null(stmt, 15),
            get_text_or_null(stmt, 16),
            get_text_or_null(stmt, 17),
            get_text_or_null(stmt, 18),
            get_text_or_null(stmt, 19)
        );
        
        i++;
    }

    *n = count;

    sqlite3_finalize(stmt);
    return 0;
}

#ifdef _WIN32

#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

    //https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
    // On unload from the virtual address space close the database connection if was open
    // TODO: put some guards on this and find way to make it work on linux(?)
    if (fdwReason == DLL_PROCESS_DETACH) close_db();
    return TRUE;
}

#endif