ROOT_DIR = ..
SRC_DIR = $(ROOT_DIR)/src
OBJ_DIR = $(ROOT_DIR)/obj
NACL_DIR = $(ROOT_DIR)/lib/nacl-20110221/build/ThinkPadL440
NACL_INC_DIR = $(NACL_DIR)/include/amd64
NACL_LIB_DIR = $(NACL_DIR)/lib/amd64
THEFT_DIR = $(ROOT_DIR)/lib/theft
INC_DIRS = . $(SRC_DIR) $(NACL_INC_DIR) $(THEFT_DIR)

CC = clang

CFLAGS  = -g -std=c99
CFLAGS += -Weverything -Wno-missing-prototypes -Wno-padded -Wno-vla -Werror
CFLAGS += -DLIMB_BITS=64 -USUPERCOP_BUILD
CFLAGS += $(addprefix -I,$(INC_DIRS))

LDFLAGS = \
-L$(ROOT_DIR) -lnrqcmdpc \
-L$(THEFT_DIR) -ltheft \
-L$(NACL_LIB_DIR) -lnacl

OBJS = \
$(OBJ_DIR)/util.o \
$(NACL_LIB_DIR)/randombytes.o

TESTS = debug

.PHONY: all
all: $(TESTS)

debug: debug.c $(OBJS) $(ROOT_DIR)/libnrqcmdpc.a
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test_%: $(OBJ_DIR)/test_%.o $(OBJS) $(ROOT_DIR)/libnrqcmdpc.a
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/util.o: util.c util.h
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR)/test_%.o: test_%.c util.h
	$(CC) -c $(CFLAGS) -o $@ $<

$(ROOT_DIR)/libnrqcmdpc.a:
	$(MAKE) -C $(ROOT_DIR) $(notdir $@)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)/test_*.o $(OBJ_DIR)/util.o
	rm -rf $(TESTS)
