CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -iquote include

TARGET = shellforge

SRC = src/history.c \
      src/lexer.c \
      src/main.c \
      src/token.c

$(TARGET): $(SRC)
	gcc $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

clean:
	rm -f $(TARGET)
