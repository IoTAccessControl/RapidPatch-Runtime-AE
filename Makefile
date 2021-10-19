# https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f

.PHONY: clean debug

OUT_DIR = _build
PROJ = iotpatch
PROJ_DIR = $(abspath .)
TARGET =  $(OUT_DIR)/$(PROJ).elf

INCLUDE_PATH += \
	-I$(PROJ_DIR)/ \
	-I$(PROJ_DIR)/app \
	-I$(PROJ_DIR)/hotpatch \
	-I$(PROJ_DIR)/libebpf/src \
	-I$(PROJ_DIR)/libebpf \

# todo use wildcard
# code files
SRC += $(PROJ_DIR)/main.c \
	$(PROJ_DIR)/app/dummy_cve.c \
	$(PROJ_DIR)/app/ihp_cli.c \
	$(PROJ_DIR)/app/ihp_porting.c \
	$(PROJ_DIR)/hotpatch/src/iotpatch.c \
	$(PROJ_DIR)/hotpatch/src/hw_bp.c \
	$(PROJ_DIR)/hotpatch/src/ebpf_test.c \
	$(PROJ_DIR)/hotpatch/src/utils.c \
	$(PROJ_DIR)/hotpatch/src/profiling.c \
	$(PROJ_DIR)/hotpatch/src/fixed_patch_points.c \
	$(PROJ_DIR)/hotpatch/src/dynamic_patch_load.c \
	$(PROJ_DIR)/hotpatch/src/patch_service.c \
	$(PROJ_DIR)/libebpf/src/ebpf_allocator.c \
	$(PROJ_DIR)/libebpf/src/hashmap.c \
	$(PROJ_DIR)/libebpf/src/ebpf_vm.c \
	$(PROJ_DIR)/libebpf/src/jit.c \
	$(PROJ_DIR)/libebpf/src/jit_thumb2.c
# 	$(PROJ_DIR)/libebpf/src/ebpf_env.c \



DEFINES += -DLINUX_TEST
VPATH := app:hotpatch/src:libebpf/src
CFLAGS += $(INCLUDE_PATH) $(DEFINES) -g
DEPFLAGS = -MT $@ -MD -MP -MF $(OUT_DIR)/deps/$(*F).d
LDFLAGS := -pthread -g


SRC_FILES = $(notdir $(SRC))
OBJ := $(SRC_FILES:.c=.o)
OBJ := $(addprefix $(OUT_DIR)/objs/, $(OBJ))

EBPF_CODE := $(notdir $(wildcard $(PROJ_DIR)/update_server/patch_code/*.ebpf.c))
EBPF_CODE := $(EBPF_CODE:.ebpf.c="")

all: debug

debug: 
	@echo "Compiling:" $(TARGET)
	make $(TARGET)
	@echo "Runing:" $(TARGET)
	@make --no-print-directory run


patch:
	@echo "Make ebpf patch"
	@$(foreach FILE, ${EBPF_CODE}, cd $(PROJ_DIR)/update_server && ./compile.sh ${FILE})

server:
	@echo "Start update server"
	sudo python3 update_server/server.py

$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	mkdir -p $(OUT_DIR)/objs
	mkdir -p $(OUT_DIR)/deps

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(OUT_DIR)/objs/%.o: %.c | $(OUT_DIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

run:
	@./$(TARGET)

clean:
	@echo "make clean"
	rm -rf $(OUT_DIR)