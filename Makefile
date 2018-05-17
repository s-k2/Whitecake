DIRS    := src src/Helper src/Win32
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS    := $(patsubst %.cpp, %.o, $(SOURCES))
OBJS    := $(foreach o,$(OBJS),./bin/$(o))
DEPFILES:= $(patsubst %.o, %.P, $(OBJS))

INCLUDE = "-Iexpat/src/"
LIBS =  -lComctl32 -lDbgHelp -lgdiplus -lSetupApi -lkernel32 -luser32 -lgdi32 -lwinspool  -lcomdlg32 -ladvapi32 -lshell32  -lole32  -loleaut32  -luuid  -lodbc32  -lodbccp32 -luxtheme -Lexpat -lexpat

WHITECAKE_BOARD := ARDUINO

CFLAGS   = -std=c++11 -Wall -O3 -fno-omit-frame-pointer -MMD -c -DWIN32 -DTRANSLATE_DE  -DWHITECAKE_FOR_$(WHITECAKE_BOARD) $(INCLUDE)
LFLAGS   = -static -static-libgcc -static-libstdc++ -Wl,-subsystem,windows -O3 $(LIBS)
# debugging flags
#CFLAGS   = -g -std=c++11 -Wall -fno-omit-frame-pointer -MMD -c -DWIN32 -DTRANSLATE_DE -DWHITECAKE_FOR_$(WHITECAKE_BOARD)  $(INCLUDE)
#LFLAGS   = -static -static-libgcc -static-libstdc++ $(LIBS)
COMPILER = g++

#link the executable
bin/whitecake: $(OBJS) bin/src/Win32/WhitecakeRes.o
	$(COMPILER) $(OBJS) bin/src/Win32/WhitecakeRes.o $(LFLAGS) -o bin/whitecake

#generate dependency information and compile
bin/%.o : %.cpp
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -o $@ -MF bin/$*.P $<
		
bin/src/Win32/WhitecakeRes.o: src/Win32/Whitecake.rc src/Win32/Whitecake.manifest
	windres src/Win32/Whitecake.rc bin/src/Win32/WhitecakeRes.o

#remove all generated files
clean:
	rm -rf bin

#include the dependency information
-include $(DEPFILES)
