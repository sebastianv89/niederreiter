SRC_DIR = src
OBJ_DIR = obj
TEST_DIR = test
NACL_DIR = lib/nacl-20110221/build/ThinkPadL440
NACL_INC_DIR = $(NACL_DIR)/include/amd64
NACL_LIB_DIR = $(NACL_DIR)/lib/amd64
SC_DIR = $(OBJ_DIR)/supercop
SC_REF_DIR = $(SC_DIR)/crypto_encrypt/nr_qc_mdpc/ref
INC_DIRS = $(SRC_DIR) $(NACL_INC_DIR)

CC = clang

CFLAGS  = -g -std=gnu11
CFLAGS += -Weverything -Wno-padded -Wdocumentation -Werror -Wpedantic -Wall -Wextra
#CFLAGS += -O3 -fomit-frame-pointer
CFLAGS += -DLIMB_BITS=64 -USUPERCOP_BUILD
CFLAGS += $(addprefix -I,$(INC_DIRS))

LDFLAGS = -L$(NACL_LIB_DIR) -lnacl

SOURCES = $(wildcard $(SRC_DIR)/*.c)
DEPS = $(SOURCES:.c=.h) $(SRC_DIR)/config.h $(SRC_DIR)/types.h
OBJS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SC_FILES = $(SOURCES) $(DEPS) $(SRC_DIR)/api.h

ARCHIVE = libnrqcmdpc.a
SUPERCOP = supercop.tar.gz

.PHONY: all
all: $(ARCHIVE) $(SUPERCOP) check

$(ARCHIVE): $(OBJS)
	ar scr $@ $^

$(SUPERCOP): $(SC_FILES:$(SRC_DIR)/%=$(SC_REF_DIR)/%)
	(cd $(SC_DIR) && tar cvzf $@ *)
	mv $(SC_DIR)/$@ ./

$(SC_REF_DIR)/%: $(SRC_DIR)/% | $(SC_REF_DIR)
	-unifdef -DSUPERCOP_BUILD -o $@ $<

$(SC_REF_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: check
check:
	$(MAKE) -C $(TEST_DIR)

.PHONY: clean
clean:
	$(MAKE) -C $(TEST_DIR) clean
	rm -rf $(OBJS)
	rm -rf $(ARCHIVE)
	rm -rf $(SC_DIR) supercop.tar.gz
