FLAGS=-Wall -I.
DEPS=util.h gitdex.h

all: demo
.PHONY: all

run: demo
	./demo

demo: demo.c $(DEPS)
	gcc $(FLAGS) $< -o $@

clean:
	rm *.o
	rm ./demo
.PHONY: clean
