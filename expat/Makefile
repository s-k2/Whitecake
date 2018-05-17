DIRS    := src
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS    := $(patsubst %.c, %.o, $(SOURCES))
OBJS    := $(foreach o,$(OBJS),./obj/$(o))
DEPFILES:= $(patsubst %.o, %.P, $(OBJS))

INCLUDE = 
LIBS =  

CFLAGS   = -DWIN32 -DXML_STATIC -fpermissive  $(INCLUDE)
LFLAGS   = rcs
COMPILER = g++

#link the executable
libexpat.a: $(OBJS)
	ar $(LFLAGS) libexpat.a $(OBJS)

#generate dependency information and compile
obj/%.o : %.c
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -o $@ -MF obj/$*.P $<
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < obj/$*.P >> obj/$*.P;

#remove all generated files
clean:
	rm -f analyze
	rm -rf obj

#include the dependency information
-include $(DEPFILES)
