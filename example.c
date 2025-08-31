#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_IMPLEMENTATION
#include "csv.h"

int
main (int argc, char *argv[])
{
    if (argc <= 1)
    {
        fprintf (stderr, "usage: %s [csv file path]\n", argv[0]);
        return 1;
    }

    FILE *csvfp = fopen (argv[1], "r");
    if (!csvfp)
    {
        perror ("fopen()");
        return 1;
    }

    CSV csv = { 0 };
    if (csv_init (&csv, csvfp, .delimiter = ',', .trim_whitespace = 1) == CSV_ERR)
    {
        fclose (csvfp);
        perror ("csv_init");
        return 1;
    }

    size_t *col_widths = NULL;
    size_t cols_cap = 0;
    size_t cur_col = 0;

    const char *data;
    size_t len;
    int st;

    while ((st = csv_next (&csv, &data, &len)) != CSV_ERR && st != CSV_EOF)
    {
        if (cur_col >= cols_cap)
        {
            size_t newcap = cols_cap ? cols_cap * 2 : 8;
            while (newcap <= cur_col)
                newcap *= 2;
            size_t *tmp = realloc (col_widths, newcap * sizeof *tmp);
            if (!tmp)
            {
                perror ("realloc");
                csv_release (&csv);
                free (col_widths);
                fclose (csvfp);
                return 1;
            }
            for (size_t i = cols_cap; i < newcap; ++i)
                tmp[i] = 0;
            col_widths = tmp;
            cols_cap = newcap;
        }

        if (len > col_widths[cur_col])
            col_widths[cur_col] = len;

        if (st == CSV_ROW_END)
            cur_col = 0;
        else
            ++cur_col;
    }

    if (st == CSV_ERR)
    {
        perror ("csv_next (first pass)");
        csv_release (&csv);
        free (col_widths);
        fclose (csvfp);
        return 1;
    }

    rewind (csvfp);
    if (csv_init (&csv, csvfp, .delimiter = ',', .trim_whitespace = 1) == CSV_ERR)
    {
        perror ("csv_init (second pass)");
        free (col_widths);
        fclose (csvfp);
        return 1;
    }

    cur_col = 0;
    int saw_any_field_in_row = 0;
    while ((st = csv_next (&csv, &data, &len)) != CSV_ERR && st != CSV_EOF)
    {
        if (cur_col >= cols_cap)
        {
            size_t newcap = cols_cap ? cols_cap * 2 : 8;
            while (newcap <= cur_col)
                newcap *= 2;
            size_t *tmp = realloc (col_widths, newcap * sizeof *tmp);
            if (!tmp)
            {
                perror ("realloc (second pass)");
                csv_release (&csv);
                free (col_widths);
                fclose (csvfp);
                return 1;
            }
            for (size_t i = cols_cap; i < newcap; ++i)
                tmp[i] = 0;
            col_widths = tmp;
            cols_cap = newcap;
        }

        int width = (int)col_widths[cur_col];
        int precision = (len > INT_MAX) ? INT_MAX : (int)len;
        printf ("%-*.*s", width, precision, data ? data : "");
        if (st != CSV_ROW_END)
            fputs ("  ", stdout);

        saw_any_field_in_row = 1;

        if (st == CSV_ROW_END)
        {
            putchar ('\n');
            cur_col = 0;
            saw_any_field_in_row = 0;
        }
        else
            ++cur_col;
    }

    if (st == CSV_EOF && saw_any_field_in_row)
        putchar ('\n');

    if (st == CSV_ERR)
        perror ("csv_next (second pass)");

    csv_release (&csv);
    free (col_widths);
    fclose (csvfp);
    return (st == CSV_ERR) ? 1 : 0;
}
