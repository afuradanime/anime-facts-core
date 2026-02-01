#include <math.h>
#include <stdio.h>
#include <windows.h>

#include "../include/anime_facts_api.h"

#define MAX_CALLS 1000

double get_time()
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/(double)f.QuadPart;
}

typedef void (*benchmark_fn)(size_t iteration, void* ctx);

void benchmark(const char* name, size_t max_calls, benchmark_fn fn, void* ctx) {
    printf("Running %zu iterations for %s...\n", max_calls, name);

    double total_time = 0.0;
    double min_time = INFINITY;
    double max_time = 0.0;

    for (size_t i = 0; i < max_calls; i++) {
        double start = get_time();
        fn(i, ctx);
        double elapsed = get_time() - start;

        total_time += elapsed;
        if (elapsed < min_time) min_time = elapsed;
        if (elapsed > max_time) max_time = elapsed;
    }

    printf("%s:\n", name);
    printf("\tAverage: %.6f ms\n", (total_time / max_calls) * 1000.0);
    printf("\tMin:     %.6f ms\n", min_time * 1000.0);
    printf("\tMax:     %.6f ms\n\n", max_time * 1000.0);
}

typedef struct {
    const char* query;
    pageable_t page;
    unsigned int n;
    partial_anime_t* data;
} fetch_from_query_ctx_t;

void bench_fetch_anime_from_query(size_t i, void* ctx) {

}

typedef struct {
    anime_t anime;
} fetch_by_id_ctx_t;

void bench_fetch_anime_by_id(size_t i, void* ctx) {
    fetch_by_id_ctx_t* c = ctx;
    fetch_anime_by_id((i % 100) + 1, &c->anime);
    free_anime(&c->anime);
}

typedef struct {
    partial_anime_t* anime;
} fetch_this_season_ctx_t;

void bench_fetch_anime_this_season(size_t i, void* ctx) {
    fetch_this_season_ctx_t* c = ctx;
    unsigned int n;
    fetch_anime_this_season(&n, &c->anime);
    free_partial_anime_array(c->anime, n);
}

int main(void) {

    set_database_path("../anime.db");

    fetch_by_id_ctx_t ctx;
    benchmark(
        "fetch_anime_by_id",
        MAX_CALLS,
        bench_fetch_anime_by_id,
        &ctx
    );

    fetch_this_season_ctx_t ctx2;
        benchmark(
        "fetch_anime_this_season",
        MAX_CALLS,
        bench_fetch_anime_this_season,
        &ctx2
    );

    fetch_from_query_ctx_t ctx3 = {
        .query = "a",
        .page = {0, 20},
        .n = 0,
        .data = NULL
    };
}