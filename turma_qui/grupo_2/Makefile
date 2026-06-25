CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude
SRC = src/main.c src/utils.c src/entities.c src/board.c src/simulation.c
OUT = ecosystem

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT) $(OUT).exe

.PHONY: all run clean
