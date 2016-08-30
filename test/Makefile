CC = gcc

SRC_DIR = ../src
OBJ_DIR = ../obj

NACL_DIR = ../lib/nacl-20110221/build/ThinkPadL440
NACL_INC_DIR = $(NACL_DIR)/include/amd64
NACL_LIB_DIR = $(NACL_DIR)/lib/amd64

INC_DIRS = -I$(SRC_DIR) -I$(NACL_INC_DIR)
LIBS = -L$(NACL_LIB_DIR) -lnacl

CFLAGS = -g -std=c99 -Wall $(INC_DIRS)
# CFLAGS = -g -std=c99 -Weverything -Werror-I$(INC_DIR)

poly: \
./poly.c \
$(SRC_DIR)/poly.c \
$(SRC_DIR)/config.h \
$(NACL_INC_DIR)/randombytes.h \
$(NACL_LIB_DIR)/randombytes.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

.PHONY: clean
clean:
	rm -rf poly
