OBJS=main.o

LIBS=
FLAGS=-Wall -I.
DEPS=util.h
TARGET=gitdex

all: $(TARGET)
.PHONY: all

run: $(TARGET)
	./$(TARGET)
.PHONY: run

clean:
	rm *.o
	rm $(TARGET)
.PHONY: clean

$(TARGET): $(OBJS)
	gcc $(FLAGS) -o $@ $< $(LIBS)

%.o: %.c $(DEPS)
	gcc $(FLAGS) -c -o $@ $<
