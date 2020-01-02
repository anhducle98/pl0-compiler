# Simple recursive descent parser for PL/0

# Build
  `$ ./make.sh`

# Usage
## Compile to assembly
    $ ./pl0 {source-file} [output-file]

## Run using interpreter
    $ ./interpreter {output-file}

# Example
    $ ./pl0 tests/sort.pl0 sort.asm
    $ ./interpreter sort.asm
