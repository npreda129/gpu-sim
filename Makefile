CC := gcc

INCFLAGS = -Iinc
LDFLAGS := -lSDL2
CFLAGS := -std=c11 -g -O

SRCS := driver.c core.c lane.c
SRCS := $(addprefix src/, $(SRCS))
OBJS := $(patsubst src/%.c,obj/%.o,$(SRCS))

all: dirs build/asm build/color

dirs:
	mkdir -p obj build

build/asm: tools/asm.c 
	$(CC) $^ -o $@ $(CFLAGS) $(INCFLAGS)

build/color: demos/color.c $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(INCFLAGS) $(LDFLAGS)

# build/%_kernel: demos/%_kernel.s
# 	build/asm $< $@

obj/%.o: src/%.c 
	$(CC) -c $< -o $@ $(CFLAGS) $(INCFLAGS)

clean:
	rm -r obj build

.PHONY: all dirs clean
