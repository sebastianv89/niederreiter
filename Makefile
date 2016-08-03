C=clang

CFLAGS=-std=c99 -g -Weverything -Werror

.PHONY: all
all: 
	@echo TODO

.PHONY: debug
debug: CFLAGS+=-g
debug: build

.PHONY: release
release: CFLAGS+=-O3
release: build

.PHONY: check
check: 
	@echo TODO

.PHONY: clean
clean:
	rm -rf build
