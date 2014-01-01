CC=gcc
RM=rm -f

CFLAGS=-Wall
LFLAGS=-L/usr/lib -lm -lGL -lglut

EXEC=plot
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)

clean:
	$(RM) $(OBJS)

mrproper: clean
	$(RM) $(EXEC)
