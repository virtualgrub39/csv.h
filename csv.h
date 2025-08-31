#ifndef _CSV_H
#define _CSV_H

#include <stddef.h>
#include <stdio.h>

struct csv_config
{
    char delimiter;       // default: ','
    int skip_header;      // skips the first valid line
    size_t max_field_len; // 0 = unlimited
};

typedef struct
{
    char *buf;
    long buf_off;
    size_t buf_len;
    size_t buf_cap;
    FILE *fp;
    struct csv_config cfg;
} CSV;

enum
{
    CSV_OK = 0,
    CSV_ROW_END = 1,
    CSV_EOF = 2,
    CSV_ERR = -1,
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

#ifdef CSV_IMPLEMENTATION

#include <errno.h>
#include <stdlib.h>
#include <string.h>

int
csv_init_opt (CSV *c, FILE *fp, struct csv_config cfg)
{
    if (!c || !fp)
    {
        errno = EINVAL;
        return CSV_ERR;
    }

    c->buf = NULL;
    c->buf_cap = 0;
    c->buf_off = 0;
    c->fp = fp;
    c->cfg = cfg;

    return CSV_OK;
}

int
csv_next (CSV *c, const char **out_data, size_t *out_len)
{
    if (!c)
    {
        errno = EINVAL;
        return CSV_ERR;
    }

    if (!c->buf)
    {
        size_t default_bufcap = 32;
        if (c->cfg.max_field_len > 0 && c->cfg.max_field_len < default_bufcap)
            default_bufcap = c->cfg.max_field_len;

        c->buf = malloc(default_bufcap + 1);
        if (!c->buf)
            return CSV_ERR;

        c->buf_cap = default_bufcap;
        c->buf_len = 0;
        c->buf_off = 0;
        c->buf[0] = '\0';
    }

    for (;;)
    {
        if (c->buf_off > 0 && c->buf_len == c->buf_cap)
        {
            size_t rem = c->buf_len - c->buf_off;
            if (rem > 0)
            {
                memmove(c->buf, c->buf + c->buf_off, rem);
            }
            c->buf_len = rem;
            c->buf_off = 0;
            c->buf[c->buf_len] = '\0';
        }

        if (c->buf_len < c->buf_cap && !feof(c->fp))
        {
            size_t to_read = c->buf_cap - c->buf_len;
            size_t n = fread(c->buf + c->buf_len, 1, to_read, c->fp);
            if (n < to_read && ferror(c->fp))
                return CSV_ERR;
            c->buf_len += n;
            c->buf[c->buf_len] = '\0';
        }

        size_t len = 0;
        int found_delim = 0;
        int found_newline = 0;
        int crlf = 0;

        while (c->buf_off + len < c->buf_len)
        {
            char ch = c->buf[c->buf_off + len];
            if (ch == c->cfg.delimiter)
            {
                found_delim = 1;
                break;
            }
            if (ch == '\n')
            {
                found_newline = 1;
                break;
            }
            if (ch == '\r')
            {
                if (c->buf_off + len + 1 < c->buf_len && c->buf[c->buf_off + len + 1] == '\n')
                {
                    found_newline = 1;
                    crlf = 1;
                    break;
                }
                found_newline = 1;
                break;
            }
            ++len;
        }

        if (found_delim || found_newline)
        {
            if (out_data) *out_data = c->buf + c->buf_off;
            if (out_len)  *out_len  = len;

            size_t consume = len + 1;
            if (found_newline && crlf) consume = len + 2;

            c->buf_off += consume;

            if (c->buf_off >= (long)c->buf_len)
            {
                c->buf_len = 0;
                c->buf_off = 0;
                c->buf[0] = '\0';
            }

            return found_newline ? CSV_ROW_END : CSV_OK;
        }

        if (feof(c->fp))
        {
            if ((long)c->buf_len > c->buf_off)
            {
                if (out_data) *out_data = c->buf + c->buf_off;
                if (out_len)  *out_len  = c->buf_len - c->buf_off;

                c->buf_len = 0;
                c->buf_off = 0;
                c->buf[0] = '\0';

                return CSV_OK;
            }
            return CSV_EOF;
        }

        if (c->cfg.max_field_len > 0 && c->buf_cap == c->cfg.max_field_len)
        {
            errno = ENOSPC;
            return CSV_ERR;
        }

        size_t new_cap = c->buf_cap * 2;
        if (c->cfg.max_field_len > 0 && c->cfg.max_field_len < new_cap)
            new_cap = c->cfg.max_field_len;

        char *tmp = realloc(c->buf, new_cap + 1);
        if (!tmp)
            return CSV_ERR;
        c->buf = tmp;
        c->buf_cap = new_cap;
    }
}

void
csv_release (CSV *c)
{
    if (!c)
        return;
    if (c->buf)
    {
        free (c->buf);
        c->buf = NULL;
        c->buf_cap = 0;
    }
}

#endif /// CSV_IMPLEMENTATION
#endif
