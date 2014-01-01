CC=gcc
RM=rm -f

CFLAGS=-Wall -Wextra -std=c99
LFLAGS=-L/usr/lib -lm -lGL -lglut

EXEC=plot
SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

$(EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)

clean:
	$(RM) $(OBJS)

mrproper: clean
	$(RM) $(EXEC)
