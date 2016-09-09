SRC_DIR = ..
OBJ_DIR = ../../obj
NACL_DIR = ../../lib/nacl-20110221/build/ThinkPadL440
NACL_INC_DIR = $(NACL_DIR)/include/amd64
NACL_LIB_DIR = $(NACL_DIR)/lib/amd64
INC_DIRS = $(SRC_DIR) $(NACL_INC_DIR)

CC = clang

CFLAGS  = -g -std=c99
CFLAGS += -Weverything -Wno-missing-prototypes
CFLAGS += -DWORD_BITS=64 -DOQS_BUILD
CFLAGS += $(addprefix -I,$(INC_DIRS))

LDFLAGS = -L$(NACL_LIB_DIR)
LDLIBS = -lnacl

OBJS = $(poly_sparse poly error pack kem dem encrypt kex:%,$(OBJ_DIR)/%.o)

TESTS = \
test_poly_sparse \
test_poly \
test_kem

.PHONY: all
all: $(TESTS)

test_%: $(OBJ_DIR)/test_%.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(OBJ_DIR)/test_%.o: test_%.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)/test_*.o
	rm -rf $(TESTS)
