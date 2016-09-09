SRC_DIR = src
OBJ_DIR = obj
TEST_DIR = $(SRC_DIR)/test
NACL_DIR = ./lib/nacl-20110221/build/ThinkPadL440
NACL_INC_DIR = $(NACL_DIR)/include/amd64
NACL_LIB_DIR = $(NACL_DIR)/lib/amd64
SC_DIR = $(OBJ_DIR)/supercop
SC_REF_DIR = $(SC_DIR)/crypto_encrypt/nr_qc_mdpc/ref
INC_DIRS = $(SRC_DIR) $(NACL_INC_DIR)

CC = clang

CFLAGS  = -g -std=c99
CFLAGS += -Weverything -Wno-padded -Wdocumentation
#CFLAGS += -O3 -fomit-frame-pointer
CFLAGS += -DWORD_BITS=64 -USUPERCOP_BUILD
CFLAGS += $(addprefix -I,$(INC_DIRS))

LDFLAGS = -L$(NACL_LIB_DIR)
LDLIBS = -lnacl

SOURCES = $(wildcard $(SRC_DIR)/*.c)
DEPS = $(SOURCES:.c=.h) $(SRC_DIR)/config.h
OBJS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SC_FILES = $(SOURCES) $(DEPS) $(SRC_DIR)/api.h

ARCHIVE = libnrqcmdpc.a
SUPERCOP = supercop.tar.gz

.PHONY: all
all: $(ARCHIVE) $(SUPERCOP)

$(ARCHIVE): $(OBJS)
	echo $(OBJS)
	ar cr $@ $^
	ranlib $@

$(SUPERCOP): $(SC_FILES:$(SRC_DIR)/%=$(SC_REF_DIR)/%)
	(cd $(SC_DIR) && tar cvzf $@ *)
	mv $(SC_DIR)/$@ ./

$(SC_REF_DIR)/%: $(SRC_DIR)/% | $(SC_REF_DIR)
	unifdef -DSUPERCOP_BUILD $< | sed 's/WORD_BITS/64/g' > $@

$(SC_REF_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: check
check: $(OBJS)
	$(MAKE) -C $(TEST_DIR)

.PHONY: clean
clean:
	$(MAKE) -C $(TEST_DIR) clean
	rm -rf $(OBJS)
	rm -rf $(ARCHIVE)
	rm -rf $(SC_DIR) supercop.tar.gz
