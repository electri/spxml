
#--------------------------------------------------------------------

CC = gcc
AR = ar cru
CFLAGS = -Wall -D_REENTRANT -D_GNU_SOURCE -g
SOFLAGS = -shared
LDFLAGS =

LINKER = $(CC)
LINT = lint -c
RM = /bin/rm -f

ifeq ($(origin version), undefined)
	version = 0.2
endif

#--------------------------------------------------------------------

LIBOBJS = spxmlutils.o spxmlevent.o spxmlreader.o spxmlparser.o spxmlstag.o \
		spxmlnode.o spdomparser.o spdomiterator.o

TARGET =  libspxml.so \
		testpull testdom

#--------------------------------------------------------------------

all: $(TARGET)

libspxml.so: $(LIBOBJS)
	$(LINKER) $(SOFLAGS) $^ -o $@

testpull: testpull.o
	$(LINKER) $(LDFLAGS) $^ -L. -lspxml -o $@

testdom: testdom.o
	$(LINKER) $(LDFLAGS) $^ -L. -lspxml -o $@

dist: clean spxml-$(version).src.tar.gz

spxml-$(version).src.tar.gz:
	@ls | sed s:^:spxml-$(version)/: > MANIFEST
	@(cd ..; ln -s spxml spxml-$(version))
	(cd ..; tar -czvf spxml/spxml-$(version).src.tar.gz `cat spxml/MANIFEST`)
	@(cd ..; rm spxml-$(version))

clean:
	@( $(RM) *.o vgcore.* core core.* $(TARGET) )

#--------------------------------------------------------------------

# make rule
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@	

%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@	
