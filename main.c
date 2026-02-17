#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "include/anime.h"
#include "include/pageable.h"
#include "include/anime_facts_api.h"
#include "include/dynamic_array.h"
#include "include/build_dll.h"

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
API void close_db() {
    if (db_conn) {
        sqlite3_close(db_conn);
        db_conn = NULL;
    }
}

API void set_database_path(const char* new_path) {

    strncpy(DB_PATH, new_path, MAX_PATH_LEN - 1);
    DB_PATH[MAX_PATH_LEN - 1] = '\0';
}

API void free_anime(anime_t* anime) {
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

API void free_anime_array(anime_t* data, unsigned int n) {
    
    for (size_t i = 0; i < n; i++) free_anime(&data[i]);
    free(data);
}

API void free_partial_anime(partial_anime_t* anime) {
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

API void free_partial_anime_array(partial_anime_t* data, unsigned int n) {
    
    for (size_t i = 0; i < n; i++) free_partial_anime(&data[i]);
    free(data);
}


API void free_studio(studio_t* studio) {
    if (!studio) return;
    if (studio->name) free(studio->name);
    if (studio->url) free(studio->url);
    memset(studio, 0, sizeof(studio_t));
}

API void free_producer(producer_t* producer) {
    if (!producer) return;
    if (producer->name) free(producer->name);
    if (producer->type) free(producer->type);
    if (producer->url) free(producer->url);
    memset(producer, 0, sizeof(producer_t));
}

API void free_licensor(licensor_t* licensor) {
    if (!licensor) return;
    if (licensor->name) free(licensor->name);
    if (licensor->type) free(licensor->type);
    if (licensor->url) free(licensor->url);
    memset(licensor, 0, sizeof(licensor_t));
}

API season_t current_season() {

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

// Helper function to count and parse the predictable pattern of anime queries
static int parse_partial_anime_query(sqlite3_stmt* stmt, unsigned int* count, partial_anime_t** data) {
    
    // Allocate array
    if (*count > 0) {
        *data = (partial_anime_t*) calloc(*count, sizeof(partial_anime_t));
        if (*data == NULL) {
            log_msg(stderr, "Memory allocation failed\n");
            sqlite3_finalize(stmt);
            return 1;
        }
    }

    // Populate array
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
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

    return 0;
}

static unsigned char map_tag_type(const char* type) {
    if (!type) return TAG_GENRE;
    if (strcmp(type, "genre") == 0) return TAG_GENRE;
    if (strcmp(type, "theme") == 0) return TAG_THEME;
    if (strcmp(type, "demographic") == 0) return TAG_DEMOGRAPHIC;
    return TAG_EXPLICIT_GENRE;
}

API int fetch_anime_from_query(const char* name, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** data) {

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

    // Get total page count for client side pagination
    sqlite3_stmt* count_stmt;
    int count_prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT COUNT(*)
        FROM anime a
        WHERE a.title LIKE ?
    ), -1, &count_stmt, 0);

    if (count_prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare count statement rc:%d errMsg %s\n", count_prep_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int count_bind_rc = sqlite3_bind_text(count_stmt, 1, pattern, -1, NULL);
    if (count_bind_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to bind name filter for count statement rc:%d errMsg %s\n", count_bind_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_step_rc = sqlite3_step(count_stmt);
    if (count_step_rc != SQLITE_ROW) {
        log_msg(stderr, "Failed to execute count statement rc:%d errMsg %s\n", count_step_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int total_count = sqlite3_column_int(count_stmt, 0);
    *total = (total_count + page.page_size - 1) / page.page_size; // Calculate total pages

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    *n = (unsigned int) count;

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, pattern, -1, NULL);
    sqlite3_bind_int(stmt, 2, page.page_size);
    sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);

    int r_c = parse_partial_anime_query(stmt, n, data);
    if (r_c != 0) {
        log_msg(stderr, "Failed to parse anime from query\n");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_finalize(count_stmt);
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

API int fetch_anime_by_id(unsigned int id, anime_t* data) {
    
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

API int fetch_anime_this_season(pageable_t page, unsigned int *n, unsigned int *total, partial_anime_t **data) {
    
    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    *n = 0;
    *data = NULL;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime a
        WHERE a.year = ? AND a.season = ?
        ORDER BY a.quality_score DESC, a.title ASC
        LIMIT ? OFFSET ?
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

    int bind_rc3 = sqlite3_bind_int(stmt, 3, page.page_size);
    if (bind_rc3 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind page size filter rc:%d errMsg %s\n", bind_rc3, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int bind_rc4 = sqlite3_bind_int(stmt, 4, page.page_number * page.page_size);
    if (bind_rc4 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind page number filter rc:%d errMsg %s\n", bind_rc4, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    // Get total page count for client side pagination
    sqlite3_stmt* count_stmt;
    int count_prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT COUNT(*)
        FROM anime a
        WHERE a.year = ? AND a.season = ?
    ), -1, &count_stmt, 0);

    if (count_prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare count statement rc:%d errMsg %s\n", count_prep_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int count_bind_rc1 = sqlite3_bind_int(count_stmt, 1, cur_season.year);
    if (count_bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind year for count statement rc:%d errMsg %s\n", count_bind_rc1, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_bind_rc2 = sqlite3_bind_text(count_stmt, 2, season_names[cur_season.season], -1, NULL);
    if (count_bind_rc2 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind season for count statement rc:%d errMsg %s\n", count_bind_rc2, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_step_rc = sqlite3_step(count_stmt);
    if (count_step_rc != SQLITE_ROW) {
        log_msg(stderr, "Failed to execute count statement rc:%d errMsg %s\n", count_step_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int total_count = sqlite3_column_int(count_stmt, 0);
    *total = (total_count + page.page_size - 1) / page.page_size; // Calculate total pages

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    *n = (unsigned int) count;

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, cur_season.year);
    sqlite3_bind_text(stmt, 2, season_names[cur_season.season], -1, NULL);

    int r_c = parse_partial_anime_query(stmt, n, data);
    if (r_c != 0) {
        log_msg(stderr, "Failed to parse anime this season query\n");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_finalize(count_stmt);
    return 0;
}

static int fetch_anime_from_studio_id(unsigned int studio_id, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** data) {

    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime_studios _as
        JOIN anime a ON a.id = _as.anime_id
        WHERE _as.studio_id = ?
        ORDER BY a.quality_score DESC, a.title ASC
        LIMIT ? OFFSET ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    int bind_rc1 = sqlite3_bind_int(stmt, 1, studio_id);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind studio id rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
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

    // Get total page count for client side pagination
    sqlite3_stmt* count_stmt;
    int count_prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT COUNT(*)
        FROM anime_studios _as
        JOIN anime a ON a.id = _as.anime_id
        WHERE _as.studio_id = ?
    ), -1, &count_stmt, 0);

    if (count_prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare count statement rc:%d errMsg %s\n", count_prep_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int count_bind_rc = sqlite3_bind_int(count_stmt, 1, studio_id);
    if (count_bind_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to bind studio id for count statement rc:%d errMsg %s\n", count_bind_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_step_rc = sqlite3_step(count_stmt);
    if (count_step_rc != SQLITE_ROW) {
        log_msg(stderr, "Failed to execute count statement rc:%d errMsg %s\n", count_step_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int total_count = sqlite3_column_int(count_stmt, 0);
    *total = (total_count + page.page_size - 1) / page.page_size; // Calculate total pages

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    *n = (unsigned int) count;

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, studio_id);
    sqlite3_bind_int(stmt, 2, page.page_size);
    sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);

    int r_c = parse_partial_anime_query(stmt, n, data);
    if (r_c != 0) {
        log_msg(stderr, "Failed to parse anime from studio query\n");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_finalize(count_stmt);
    return 0;
}

static int fetch_anime_from_producer_id(unsigned int producer_id, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** data) {

    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime_producers ap
        JOIN anime a ON a.id = ap.anime_id
        WHERE ap.producer_id = ?
        ORDER BY a.quality_score DESC, a.title ASC
        LIMIT ? OFFSET ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    int bind_rc1 = sqlite3_bind_int(stmt, 1, producer_id);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind producer id rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
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

    // Get total page count for client side pagination
    sqlite3_stmt* count_stmt;
    int count_prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT COUNT(*)
        FROM anime_producers ap
        JOIN anime a ON a.id = ap.anime_id
        WHERE ap.producer_id = ?
    ), -1, &count_stmt, 0);

    if (count_prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare count statement rc:%d errMsg %s\n", count_prep_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int count_bind_rc = sqlite3_bind_int(count_stmt, 1, producer_id);
    if (count_bind_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to bind producer id for count statement rc:%d errMsg %s\n", count_bind_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_step_rc = sqlite3_step(count_stmt);
    if (count_step_rc != SQLITE_ROW) {
        log_msg(stderr, "Failed to execute count statement rc:%d errMsg %s\n", count_step_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int total_count = sqlite3_column_int(count_stmt, 0);
    *total = (total_count + page.page_size - 1) / page.page_size; // Calculate total pages

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    *n = (unsigned int) count;

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, producer_id);
    sqlite3_bind_int(stmt, 2, page.page_size);
    sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);

    int r_c = parse_partial_anime_query(stmt, n, data);
    if (r_c != 0) {
        log_msg(stderr, "Failed to parse anime from producer query\n");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_finalize(count_stmt);
    return 0;
}

static int fetch_anime_from_licensor_id(unsigned int licensor_id, pageable_t page, unsigned int* n, unsigned int* total, partial_anime_t** data) {

    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime_licensors al
        JOIN anime a ON a.id = al.anime_id
        WHERE al.licensor_id = ?
        ORDER BY a.quality_score DESC, a.title ASC
        LIMIT ? OFFSET ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    int bind_rc1 = sqlite3_bind_int(stmt, 1, licensor_id);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind licensor id rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
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

    // Get total page count for client side pagination
    sqlite3_stmt* count_stmt;
    int count_prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT COUNT(*)
        FROM anime_licensors al
        JOIN anime a ON a.id = al.anime_id
        WHERE al.licensor_id = ?
    ), -1, &count_stmt, 0);

    if (count_prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare count statement rc:%d errMsg %s\n", count_prep_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int count_bind_rc = sqlite3_bind_int(count_stmt, 1, licensor_id);
    if (count_bind_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to bind licensor id for count statement rc:%d errMsg %s\n", count_bind_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_step_rc = sqlite3_step(count_stmt);
    if (count_step_rc != SQLITE_ROW) {
        log_msg(stderr, "Failed to execute count statement rc:%d errMsg %s\n", count_step_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int total_count = sqlite3_column_int(count_stmt, 0);
    *total = (total_count + page.page_size - 1) / page.page_size; // Calculate total pages

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    *n = (unsigned int) count;

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, licensor_id);
    sqlite3_bind_int(stmt, 2, page.page_size);
    sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);

    int r_c = parse_partial_anime_query(stmt, n, data);
    if (r_c != 0) {
        log_msg(stderr, "Failed to parse anime from licensor query\n");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_finalize(count_stmt);
    return 0;
}

API int fetch_studio_by_id(unsigned int id, studio_t *data, pageable_t page, unsigned int *n, unsigned int* total, partial_anime_t **anime) {

    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    // Get studio info
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT id, name, url
        FROM studios
        WHERE id = ?
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
        log_msg(stderr, "No studio found with id %u\n", id);
        sqlite3_finalize(stmt);
        return 1;  // Not found / error
    }

    data->id = sqlite3_column_int(stmt, 0);
    const char* name = get_text_or_null(stmt, 1);
    if (name) data->name = strdup(name);
    const char* url = get_text_or_null(stmt, 2);
    if (url) data->url = strdup(url);

    sqlite3_finalize(stmt);
    
    // Get anime from studio
    return fetch_anime_from_studio_id(id, page, n, total, anime);
}

API int fetch_producer_by_id(unsigned int id, producer_t *data, pageable_t page, unsigned int *n, unsigned int* total, partial_anime_t **anime) {

    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    // Get producer info
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT id, name, type, url
        FROM producers
        WHERE id = ?
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
        log_msg(stderr, "No producer found with id %u\n", id);
        sqlite3_finalize(stmt);
        return 1;  // Not found / error
    }

    data->id = sqlite3_column_int(stmt, 0);
    const char* name = get_text_or_null(stmt, 1);
    if (name) data->name = strdup(name);
    const char* type = get_text_or_null(stmt, 2);
    if (type) data->type = strdup(type);
    const char* url = get_text_or_null(stmt, 3);
    if (url) data->url = strdup(url);

    sqlite3_finalize(stmt);
    
    // Get anime from producer
    return fetch_anime_from_producer_id(id, page, n, total, anime);
}

API int fetch_licensor_by_id(unsigned int id, licensor_t *data, pageable_t page, unsigned int *n, unsigned int* total, partial_anime_t **anime) {

    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    // Get licensor info
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT id, name, type, url
        FROM licensors
        WHERE id = ?
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
        log_msg(stderr, "No licensor found with id %u\n", id);
        sqlite3_finalize(stmt);
        return 1;  // Not found / error
    }

    data->id = sqlite3_column_int(stmt, 0);
    const char* name = get_text_or_null(stmt, 1);
    if (name) data->name = strdup(name);
    const char* type = get_text_or_null(stmt, 2);
    if (type) data->type = strdup(type);
    const char* url = get_text_or_null(stmt, 3);
    if (url) data->url = strdup(url);

    sqlite3_finalize(stmt);
    
    // Get anime from licensor
    return fetch_anime_from_licensor_id(id, page, n, total, anime);
}

API int fetch_anime_from_tag(unsigned int tag_id, pageable_t page, unsigned int *n, unsigned int* total, partial_anime_t **data) {
    sqlite3* connection = get_db();
    if (!connection) return 1;
    
    sqlite3_stmt* stmt;
    int prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT a.id, a.url, a.title, a.type_id, a.source, a.episodes, a.status_id, a.airing, a.duration, a.start_date, a.end_date, a.season, a.year, a.broadcast_day, a.broadcast_time, a.broadcast_timezone, a.image_url, a.small_image_url, a.large_image_url, a.trailer_embed_url
        FROM anime_tags at
        JOIN anime a ON a.id = at.anime_id
        WHERE at.tag_id = ?
        ORDER BY a.quality_score DESC, a.title ASC
        LIMIT ? OFFSET ?
    ), -1, &stmt, 0);

    if (prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare statement rc:%d errMsg %s\n", prep_rc, sqlite3_errmsg(connection));
        return 1;
    }

    int bind_rc1 = sqlite3_bind_int(stmt, 1, tag_id);
    if (bind_rc1 != SQLITE_OK) {
        log_msg(stderr, "Failed to bind tag id rc:%d errMsg %s\n", bind_rc1, sqlite3_errmsg(connection));
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

    // Get total page count for client side pagination
    sqlite3_stmt* count_stmt;
    int count_prep_rc = sqlite3_prepare_v2(connection, SQL(
        SELECT COUNT(*)
        FROM anime_tags at
        JOIN anime a ON a.id = at.anime_id
        WHERE at.tag_id = ?
    ), -1, &count_stmt, 0);

    if (count_prep_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to prepare count statement rc:%d errMsg %s\n", count_prep_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return 1;
    }

    int count_bind_rc = sqlite3_bind_int(count_stmt, 1, tag_id);
    if (count_bind_rc != SQLITE_OK) {
        log_msg(stderr, "Failed to bind tag id for count statement rc:%d errMsg %s\n", count_bind_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int count_step_rc = sqlite3_step(count_stmt);
    if (count_step_rc != SQLITE_ROW) {
        log_msg(stderr, "Failed to execute count statement rc:%d errMsg %s\n", count_step_rc, sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        sqlite3_finalize(count_stmt);
        return 1;
    }

    int total_count = sqlite3_column_int(count_stmt, 0);
    *total = (total_count + page.page_size - 1) / page.page_size; // Calculate total pages

    // count results
    size_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    *n = (unsigned int) count;

    // Reset statement to beginning
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, tag_id);
    sqlite3_bind_int(stmt, 2, page.page_size);
    sqlite3_bind_int(stmt, 3, page.page_number * page.page_size);

    int r_c = parse_partial_anime_query(stmt, n, data);
    if (r_c != 0) {
        log_msg(stderr, "Failed to parse anime from tag query\n");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_finalize(count_stmt);
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