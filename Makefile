
OUT=out
DEPS=-lX11
FLAGS= -g
      # ${DEPS} \
      # -O3 \

# GCC=../tinycc/tcc
# GCC=tcc
GCC=gcc
# GCC=g++

.PHONY: ${OUT} clean

all: ${OUT}/main

clean:
	rm -rf ${OUT}

${OUT}/main: src/main.c | ${OUT}
	${GCC} ${FLAGS} $^ -o $@

${OUT}:
	mkdir -p $@
