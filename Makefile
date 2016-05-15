FLAGS=-Wall -I.
DEPS=util.h gitdex.h

all: demo
.PHONY: all

run: demo
	./demo .git/index

demo: demo.c $(DEPS)
	gcc $(FLAGS) $< -o $@

clean:
	rm ./demo
.PHONY: clean
