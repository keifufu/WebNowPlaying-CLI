# There is no linux32 build since who even uses 32bit on linux
CLEAN = rm -rf build
MKDIR = mkdir -p build/obj
CFLAGS = -Wall
# CFLAGS += -O2
CFLAGS += -g

ifeq ($(OS),Windows_NT)
	CLEAN = rmdir /s /q build
	MKDIR = mkdir build\obj
	CFLAGS = /EHsc
	CFLAGS += /O2
#	CFLAGS += /DEBUG
endif

all:
	@echo Available targets: linux64, win64

OBJS_LINUX64 = build/obj/wnpcli_linux_x64.o build/obj/daemon_linux_x64.o build/obj/cargs_linux_x64.o
linux64: | build
	@$(MAKE) --no-print-directory build/wnpcli_linux_x64
build/obj/%_linux_x64.o: src/%.c
	clang $(CFLAGS) -c $< -o $@
build/wnpcli_linux_x64: $(OBJS_LINUX64)
	clang $(CFLAGS) $^ -o $@ -Ldeps -lwnp_linux_amd64

OBJS_WIN64 = build/obj/wnpcli_win_x64.obj build/obj/daemon_win_x64.obj build/obj/cargs_win_x64.obj
win64: | build
	@$(MAKE) --no-print-directory build/wnpcli_win_x64.exe
build/obj/%_win_x64.obj: src/%.c
	cl $(CFLAGS) /c $< /Fo$@
build/wnpcli_win_x64.exe: $(OBJS_WIN64)
	link /MACHINE:x64 /LIBPATH:deps libwnp_win64.lib $^ /OUT:$@

build:
	$(MKDIR)
.PHONY: clean
clean:
	$(CLEAN)