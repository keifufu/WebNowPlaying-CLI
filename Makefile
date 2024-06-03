# There is no linux32 build since who even uses 32bit on linux
VERSION = $(shell cat VERSION)
CLEAN = rm -rf build
MKDIR = mkdir -p build/obj
CFLAGS = -Wall -Wno-unused-command-line-argument -Ideps -Ldeps -L$(LIB_PATH) -I$(INCLUDE_PATH)
CFLAGS += -O2 -DCLI_VERSION='"$(VERSION)"'
# CFLAGS += -g

ifeq ($(OS),Windows_NT)
	VERSION = $(shell type VERSION)
	CLEAN = rmdir /s /q build
	MKDIR = mkdir build\obj
	CFLAGS = /EHsc /Ideps
	CFLAGS += /O2 /DCLI_VERSION=\"$(VERSION)\"
#	CFLAGS += /Zi /DEBUG
#	LINK_FLAGS = /DEBUG
endif

all:
	@echo Available targets: linux64, win64

OBJS_LINUX64 = build/obj/wnpcli_linux_amd64.o build/obj/daemon_linux_amd64.o build/obj/cargs_linux_amd64.o
linux64: | build
	@$(MAKE) --no-print-directory build/wnpcli_linux_amd64
build/obj/%_linux_amd64.o: src/%.c
	clang $(CFLAGS) -c $< -o $@
build/wnpcli_linux_amd64: $(OBJS_LINUX64)
	clang $(CFLAGS) $^ -o $@ -l:libwnp.a

OBJS_WIN64 = build/obj/wnpcli_win64.obj build/obj/daemon_win64.obj build/obj/cargs_win64.obj
win64: | build
	@$(MAKE) --no-print-directory build/wnpcli_win64.exe
build/obj/%_win64.obj: src/%.c
	cl $(CFLAGS) /c $< /Fo$@
build/wnpcli_win64.exe: $(OBJS_WIN64)
	link $(LINK_FLAGS) /MACHINE:x64 /LIBPATH:deps libwnp_win64.lib $^ /OUT:$@

build:
	$(MKDIR)
.PHONY: clean
clean:
	$(CLEAN)