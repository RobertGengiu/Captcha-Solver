CC=gcc
CFLAGS=-Wall -Wextra -c
EXE=tema3

build: captcha.o
	$(CC) $^ -o $(EXE)

alcator.o: tema3.c
	$(CC) $(CFLAGS) $^ -o $@

run:
	./tema3
	
clean:
	rm -rf tema3.o $(EXE)
