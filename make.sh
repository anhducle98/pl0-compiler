mkdir build
cd src
gcc scanner.c parser.c symbol_table.c -o ../build/pl0
gcc interpreter.c -o ../build/interpreter
