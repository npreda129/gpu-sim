CC := gcc

INCFLAGS = -Iinc

CFLAGS := -std=c11 -g -O

SRCS := driver.c core.c lane.c
SRCS := $(addprefix $(SRCDIR)/, $(SRCS))
OBJS := $(patsubst src/%.c,obj/%.o,$(wildcard $(SRCS)))

all: dirs build/asm build/mandelbrot

dirs:
	mkdir -p obj build

build/asm: tools/asm.c
	$(CC) $^ -o $@ $(CFLAGS) $(INCFLAGS)

build/mandelbrot: demos/mandelbrot.c $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(INCFLAGS)

obj/%.o: src/%.c
	$(CC) $^ -o $@ $(CFLAGS) $(INCFLAGS)

clean:
	rm -r obj build
.PHONY: all dirs clean
