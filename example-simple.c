#include <stdio.h>

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
    if (csv_init (&csv, csvfp, .delimiter = ',', .skip_header = 1) == CSV_ERR)
    {
        fclose (csvfp);
        perror ("csv_init");
        return 1;
    }
    const char *data;
    size_t len;
    while (1)
    {
        int st = csv_next (&csv, &data, &len);
        if (st == CSV_ERR)
        {
            perror ("csv_next()");
            break;
        }
        else if (st == CSV_EOF)
            break;
        // if (len > 0)
        //     fwrite (data, sizeof (data[0]), len, stdout);
        // putc ('\t', stdout);
        // if (st == CSV_ROW_END)
        //     putc ('\n', stdout);
    }
    csv_release (&csv);
    fclose (csvfp);
    return 0;
}
