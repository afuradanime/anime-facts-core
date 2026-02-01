#include <stdio.h>
#include <windows.h>

#include "../include/anime_facts_api.h"

#define MAX_CALLS 100

double get_time()
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/(double)f.QuadPart;
}

void benchmark_fetch_anime_by_id(size_t max_calls) {
    
    printf("Running %zu iterations for the function: fetch_anime_by_id...\n", max_calls);

    size_t i = 0;
    anime_t* p = NULL;

    double d = get_time();
    for(;i<max_calls;i++) {
        fetch_anime_by_id(i, p);
    }

    double t = get_time() - d;
    printf("fetch_anime_by_id has an average runtime of: %f\n", t);
}

int main(void) {

    benchmark_fetch_anime_by_id(MAX_CALLS);
}