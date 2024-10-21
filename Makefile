MAKEFLAGS += --silent
CC = mold -run clang++
FLAGS = \
	-ferror-limit=1 \
	-fno-math-errno \
	-fsanitize=address \
	-fsanitize=bounds \
	-fsanitize=float-divide-by-zero \
	-fsanitize=implicit-conversion \
	-fsanitize=integer \
	-fsanitize=nullability \
	-fshort-enums \
	-ggdb \
	-march=native \
	-O3 \
	-std=c++17 \
	-Werror \
	-Weverything \
	-Wno-c11-extensions \
	-Wno-c++98-compat-pedantic \
	-Wno-c99-extensions \
	-Wno-covered-switch-default \
	-Wno-padded \
	-Wno-unsafe-buffer-usage

.PHONY: all
all: bin/main

.PHONY: clean
clean:
	rm -rf bin/

.PHONY: run
run: all
	./scripts/run.sh ex/sum_1_to_n.j2

bin/main: src/main.cpp
	mkdir -p bin/
	clang-format -i -verbose src/main.cpp
	$(CC) $(FLAGS) -o bin/main src/main.cpp
