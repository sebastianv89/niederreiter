CC = clang

SRC_DIR = src
OBJ_DIR = obj
TEST_DIR = test
NACL_DIR = ./lib/nacl-20110221/build/ThinkPadL440
NACL_INC_DIR = $(NACL_DIR)/include/amd64
NACL_LIB_DIR = $(NACL_DIR)/lib/amd64
INC_DIRS = $(SRC_DIR) $(NACL_INC_DIR)

CFLAGS  = -g -std=c99 -O3 -fomit-frame-pointer
CFLAGS += -DWORD_BITS=64
CFLAGS += -DOQS_BUILD
CFLAGS += -Weverything -Wno-missing-prototypes -Wno-padded
CFLAGS += $(INC_DIRS:%=-I%)

LDFLAGS = -L$(NACL_LIB_DIR)
LDLIBS = -lnacl

oqs_kex:
	@echo TODO: $@

supercop:
	@echo TODO: $@

encrypt: CFLAGS += -DENCRYPT_MAIN
encrypt: \
$(OBJ_DIR)/encrypt.o \
$(OBJ_DIR)/pack.o \
$(OBJ_DIR)/kem.o \
$(OBJ_DIR)/dem.o \
$(OBJ_DIR)/poly.o \
$(OBJ_DIR)/poly_sparse.o \
$(OBJ_DIR)/error.o \
$(OBJ_DIR)/debug.o \
$(NACL_LIB_DIR)/randombytes.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

kex: CFLAGS += -DKEX_MAIN
kex: \
$(OBJ_DIR)/kex.o \
$(OBJ_DIR)/pack.o \
$(OBJ_DIR)/kem.o \
$(OBJ_DIR)/dem.o \
$(OBJ_DIR)/poly.o \
$(OBJ_DIR)/poly_sparse.o \
$(OBJ_DIR)/error.o \
$(OBJ_DIR)/debug.o \
$(NACL_LIB_DIR)/randombytes.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

poly: CFLAGS += -DPOLY_MAIN
poly: \
$(OBJ_DIR)/poly.o \
$(OBJ_DIR)/debug.o \
$(NACL_LIB_DIR)/randombytes.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test_poly: \
$(OBJ_DIR)/test_poly.o \
$(OBJ_DIR)/poly.o \
$(OBJ_DIR)/debug.o \
$(NACL_LIB_DIR)/randombytes.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test_kem: \
$(OBJ_DIR)/test_kem.o \
$(OBJ_DIR)/kem.o \
$(OBJ_DIR)/pack.o \
$(OBJ_DIR)/poly.o \
$(OBJ_DIR)/poly_sparse.o \
$(OBJ_DIR)/error.o \
$(OBJ_DIR)/debug.o \
$(NACL_LIB_DIR)/randombytes.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/encrypt.o: \
$(SRC_DIR)/encrypt.c \
$(SRC_DIR)/encrypt.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h \
$(SRC_DIR)/pack.h \
$(SRC_DIR)/kem.h \
$(SRC_DIR)/dem.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/kex.o: \
$(SRC_DIR)/kex.c \
$(SRC_DIR)/kex.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h \
$(SRC_DIR)/pack.h \
$(SRC_DIR)/kem.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/pack.o: \
$(SRC_DIR)/pack.c \
$(SRC_DIR)/pack.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/kem.o: \
$(SRC_DIR)/kem.c \
$(SRC_DIR)/kem.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h \
$(SRC_DIR)/poly.h \
$(SRC_DIR)/poly_sparse.h \
$(SRC_DIR)/error.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/dem.o: \
$(SRC_DIR)/dem.c \
$(SRC_DIR)/dem.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/poly.o: \
$(SRC_DIR)/poly.c \
$(SRC_DIR)/poly.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/poly_sparse.o: \
$(SRC_DIR)/poly_sparse.c \
$(SRC_DIR)/poly_sparse.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h \
$(SRC_DIR)/poly.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/error.o: \
$(SRC_DIR)/error.c \
$(SRC_DIR)/error.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h \
$(SRC_DIR)/poly.h \
$(SRC_DIR)/poly_sparse.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/debug.o: \
$(SRC_DIR)/debug.c \
$(SRC_DIR)/debug.h \
$(SRC_DIR)/config.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/test_poly.o: \
$(TEST_DIR)/poly.c \
$(SRC_DIR)/poly.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/debug.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/test_kem.o: \
$(TEST_DIR)/kem.c \
$(SRC_DIR)/kem.h \
$(SRC_DIR)/config.h \
$(SRC_DIR)/pack.h \
$(SRC_DIR)/poly.h \
$(SRC_DIR)/poly_sparse.h \
$(SRC_DIR)/error.h \
$(SRC_DIR)/debug.h
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf encrypt kex poly
	rm -rf $(OBJ_DIR)/*.o
