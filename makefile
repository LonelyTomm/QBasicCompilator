run:
	gcc -o main main.c lexer.c parser.c -std=c11 -Wall -Wextra -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
	./main
	rm main

debug:
	gcc -g -o main main.c lexer.c parser.c
	gdb main
	rm main
