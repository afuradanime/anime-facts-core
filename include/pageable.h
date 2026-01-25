#ifndef PAGEABLE_H
#define PAGEABLE_H

struct pageable {
    unsigned short page_size;
    unsigned short page_number;
};

typedef struct pageable pageable_t;

#endif