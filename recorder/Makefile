TESTNAME	:= x2000_record_4_test

export CROSS_COMPILE := $(PWD)/../../sdkk/prebuilts/toolchains/mips-gcc720-glibc229/bin/mips-linux-gnu-

export CC:=$(CROSS_COMPILE)gcc

CFLAGS := -Wall -O2 -lpthread
CFLAGS += -I.


TARGET = $(PWD)/x2000_record_4_test

SOURCES = $(wildcard  alg/*.c sadc/*.c tf/*.c crc/*.c tcp/*.c uart/*.c hash/*.c auth/*.c *.c)
HEADERS = $(wildcard  alg/*.h sadc/*.h tf/*.h crc/*.h tcp/*.h uart/*.h hash/*.h auth/*.h *.h)

DBGOBJFILES = $(SOURCES:%.c=debug/%.o)
RELOBJFILES = $(SOURCES:%.c=%.o)

.PHONY:clean debug release install

$(TARGET): $(RELOBJFILES)
	$(LINK.c) -o $@ $^

$(RELOBJFILES): %.o: %.c
	$(COMPILE.c) -o $@ $^
install:$(TARGET)
	install -d $(EXEC_DIR)
	install $(TARGET) $(EXEC_DIR)
clean:
	$(RM) *.o $(TARGET)