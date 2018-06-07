DIRS    := src src/Helper src/Win32
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS    := $(patsubst %.cpp, %.o, $(SOURCES))
OBJS    := $(foreach o,$(OBJS),./bin/$(o))
DEPFILES:= $(patsubst %.o, %.P, $(OBJS))

INCLUDE = "-Iexpat/src/"
LIBS =  -lComctl32 -lDbgHelp -lgdiplus -lSetupApi -lkernel32 -luser32 -lgdi32 -lwinspool  -lcomdlg32 -ladvapi32 -lshell32  -lole32  -loleaut32  -luuid  -lodbc32  -lodbccp32 -luxtheme -Lexpat -lexpat
STATIC_LIBC_FLAGS = -static -static-libgcc -static-libstdc++

CFLAGS   = -std=c++11 -Wall -O3 -fno-omit-frame-pointer -MMD -c -DWIN32 -DTRANSLATE_DE  $(INCLUDE)
LFLAGS   = $(STATIC_LIBC_FLAGS) -Wl,-subsystem,windows -O3 $(LIBS)
# debugging flags
#CFLAGS   = -g -std=c++11 -Wall -fno-omit-frame-pointer -MMD -c -DWIN32 -DTRANSLATE_DE -DWHITECAKE_FOR_$(WHITECAKE_BOARD)  $(INCLUDE)
#LFLAGS   = -static -static-libgcc -static-libstdc++ $(LIBS)
COMPILER = g++

#link the executable
bin/whitecake: src/StdlibArduino.h src/StdlibTinyTick.h src/StdlibTinyBas.h $(OBJS) bin/src/Win32/WhitecakeRes.o
	$(COMPILER) $(OBJS) bin/src/Win32/WhitecakeRes.o $(LFLAGS) -o bin/whitecake

#generate dependency information and compile
bin/%.o : %.cpp
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -o $@ -MF bin/$*.P $<
		
bin/src/Win32/WhitecakeRes.o: src/Win32/Whitecake.rc src/Win32/Whitecake.manifest
	windres src/Win32/Whitecake.rc bin/src/Win32/WhitecakeRes.o

# Everything related to the Stdlib*.h-files used by the compiler
bin/ParseObjDump.exe: tools/ParseObjDump.cpp
	@mkdir -p $(@D)
	$(COMPILER) $(STATIC_LIBC_FLAGS) tools/ParseObjDump.cpp -o bin/ParseObjDump

src/StdlibArduino.h: bin/ParseObjDump.exe dep/StdlibArduino/bin/StdlibArduino.hex dep/StdlibArduino/bin/StdlibArduino.symbols
	bin/ParseObjDump.exe dep/StdlibArduino/bin/StdlibArduino >$@

dep/StdlibArduino/bin/StdlibArduino.hex: $(wildcard dep/StdlibArduino/src/*.c) $(wildcard dep/StdlibArduino/src/*.h)
	make -C dep/StdlibArduino

src/StdlibTinyTick.h: bin/ParseObjDump.exe dep/StdlibTinyTick/bin/StdlibTinyTick.hex dep/StdlibTinyTick/bin/StdlibTinyTick.symbols
	bin/ParseObjDump.exe dep/StdlibTinyTick/bin/StdlibTinyTick >$@

dep/StdlibTinyTick/bin/StdlibTinyTick.hex: $(wildcard dep/StdlibTinyTick/src/*.c) $(wildcard dep/StdlibTinyTick/src/*.h)
	make -C dep/StdlibTinyTick

src/StdlibTinyBas.h: bin/ParseObjDump.exe dep/StdlibTinyBas/bin/StdlibTinyBas.hex dep/StdlibTinyBas/bin/StdlibTinyBas.symbols
	bin/ParseObjDump.exe dep/StdlibTinyBas/bin/StdlibTinyBas >$@

dep/StdlibTinyBas/bin/StdlibTinyBas.hex: $(wildcard dep/StdlibTinyBas/src/*.c) $(wildcard dep/StdlibTinyBas/src/*.h)
	make -C dep/StdlibTinyBas

#remove all generated files
clean:
	rm -rf bin

#include the dependency information
-include $(DEPFILES)
