####### k8000 makefile
REL = 2.2.0

AR =  $(CROSS_COMPILE)ar -r
CC  = $(CROSS_COMPILE)gcc

#CFLAGS = -Wall -O3

SOURCES = k8000.c
OBJECTS = k8000.o

# 2.6 kernel module stuff
obj-m += mod_k8000.o
mod_k8000-objs := k8000.o
RTL_DIR = /usr/src/rtai/rtai-core
EXTRA_CFLAGS += -I$(RTL_DIR)/include -I/usr/include

TARGET = libk8000.a

DESTLIBDIR = /usr/lib
DESTINCDIR = /usr/include

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c .pc

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<


####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $(TARGET) $(OBJECTS)

clean:
	rm -fr $(OBJECTS) $(TARGET) $(MODTARGET) *.o *.ko mod_k8000.mod*  .mod_k8000* .tmp_versions .k8000*
	@cd ./examples; make clean

install: all
	cp -f $(TARGET) $(DESTLIBDIR)/
	cp -f k8000.h $(DESTINCDIR)/k8000.h

demos:
	@cd ./examples; make

dist: clean
	tar -C.. -cvf ./libk8000-$(REL).tar ./libk8000
	gzip libk8000-$(REL).tar
