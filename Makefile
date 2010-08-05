####### k8000 makefile
REL = 2.2.0

AR =  $(CROSS_COMPILE)ar -r
CC  = $(CROSS_COMPILE)gcc

#CFLAGS = -Wall -O3
CFLAGS = -fPIC

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

python:
	swig -python -shadow k8000.i
	gcc -fPIC -c k8000_wrap.c -I/usr/include/python2.6
	ld -shared k8000.o k8000_wrap.o -o _k8000.so

clean:
	rm -fr $(OBJECTS) $(TARGET) $(MODTARGET) *.o *.ko mod_k8000.mod*  .mod_k8000* .tmp_versions .k8000*
	rm -rf *.so *.pyc k8000.py k8000_wrap.c
	@cd ./examples; make clean

install: all
	cp -f $(TARGET) $(DESTLIBDIR)/
	cp -f k8000.h $(DESTINCDIR)/k8000.h

demos:
	@cd ./examples; make

dist: clean
	tar -C.. -cvf ./libk8000-$(REL).tar ./libk8000
	gzip libk8000-$(REL).tar

