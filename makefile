CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = shellforge

SRC = src/builtin.c \
      src/executor.c \
      src/expand.c \
      src/history.c \
      src/lexer.c \
      src/main.c \
      src/parser.c \
      src/token.c

$(TARGET): $(SRC)
	gcc $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

clean:
	rm -f $(TARGET)
