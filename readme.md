# csv.h

Minimal, single header, stream oriented CSV parser written in C99.

## Usage

Include `csv.h`. Define `CSV_IMPLEMENTATION` once per translation unit.

See [example.c](example.c) for example usage, and [header file](csv.h#L7) for configuration options.

## Performance

It walked through 2000000 line csv file in 1.75 seconds.

The example code runs in 40 seconds for the same 2 mil. row file. Most of the time is spent on printing.

## License

See [LICENSE](LICENSE)
