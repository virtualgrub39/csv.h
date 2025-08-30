#ifndef _CSV_H
#define _CSV_H

#include <stddef.h>
#include <stdio.h>

typedef struct
{
    char *buf;
    size_t off;
    FILE *fp;
} CSV;

enum
{
    CSV_OK = 0,
    CSV_ROW_END = 1,
    CSV_EOF = 2,
    CSV_ERR = -1,
};

struct csv_config
{
    char delimiter;       // default: ','
    int skip_header;      // skips the first valid line
    size_t max_field_len; // 0 = unlimited
};

// initialize CSV context; Returns < 0 on error, or 0 on success.
int csv_init_opt (CSV *c, FILE *fp, struct csv_config cfg);
// Get next column from current (or next) row
// On success, outputs cell contents into data, and it's length into len.
// This data is valid to next csv_next() or csv_release() call.
// On error, sets errno and immediately returns
// Returns currect CSV file status
int csv_next (CSV *c, const char **data, size_t *len);
// deinitialize CSV context;
void csv_release (CSV *c);

#define csv_init(c, fp, ...) csv_init_opt (c, fp, (struct csv_config){ .delimiter = ',', __VA_ARGS__ })

#define CSV_IMPLEMENTATION // FIXME: remove
#ifdef CSV_IMPLEMENTATION

int
csv_init_opt (CSV *c, FILE *fp, struct csv_config cfg)
{
    (void)c;
    (void)fp;
    (void)cfg;
    return -1;
}

int csv_next (CSV *c, const char **data, size_t *len)
{
    (void)c;
    (void)data;
    (void)len;
    return CSV_ERR;
}

void
csv_release (CSV *c)
{
    (void)c;
}

#endif /// CSV_IMPLEMENTATION
#endif
