CC = gcc

SRC_DIR = src
OBJ_DIR = obj

# Not very pretty
INC_DIRS = -I$(SRC_DIR) -I./lib/nacl-20110221/build/ThinkPadL440/include/amd64
LIBS = -L./lib/nacl-20110221/build/ThinkPadL440/lib/amd64 -lnacl

CFLAGS = -g -std=c99 -Wall $(INC_DIRS)
# CFLAGS = -g -std=c99 -Weverything -Werror-I$(INC_DIR)

# for debugging
poly: \
$(SRC_DIR)/poly.c \
$(SRC_DIR)/kem.h \
lib/nacl-20110221/build/ThinkPadL440/include/amd64/randombytes.h \
lib/nacl-20110221/build/ThinkPadL440/lib/amd64/randombytes.o
	$(CC) $(CFLAGS) -DPOLY_MAIN $^ $(LIBS) -o $@

# for debugging
word: \
$(SRC_DIR)/word.c \
$(SRC_DIR)/word.h \
lib/nacl-20110221/build/ThinkPadL440/include/amd64/randombytes.h \
lib/nacl-20110221/build/ThinkPadL440/lib/amd64/randombytes.o
#$(CC) -DSUPERCOP_BUILD -Ilib/nacl-20110221/build/ThinkPadL440/include/amd64/ -E $<
	$(CC) -E -DWORD_BITS=64 $<

.PHONY: clean
clean:
	rm -rf poly word
