STATIC=mingw-lib.a

CC = gcc
CFLAGS = -O3 -Wall -fPIC -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -static

SRC = mingw-lib.c

OBJ = $(SRC:.c=.o)

$(STATIC): $(OBJ)
	@echo "[Link (Static)]"
	@ar rcs $@ $^

.c.o:
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ)