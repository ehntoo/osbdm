BASE=$(shell uname -s)
VERSION=1.0
RELEASE=0
#OSBDM_ROOT=./osbdm
OSBDM_ROOT=.
#VPATH=$(OSBDM_ROOT)/osbdm-pc/osbdmusb_api
VPATH=.

ifeq ($(strip $(BASE)),Linux)
   HOST=linux
   CC=gcc -Wall
   ifeq ($(RELEASE),1)
     DEBUG=-O2
   else
     DEBUG=-g
   endif

   INCLUDES=-I$(OSBDM_ROOT)/osbdm-pc/osbdmusb_api -I$(OSBDM_ROOT)/osbdm-pc/osbdmusb/libusb/include -I../../../osbdm-common/src
   CFLAGS=-DLINUX=1 -DUNIX $(DEBUG) $(INCLUDES) -fPIC
   LIBS= -lbsd -lnsl -lthread -ldl -lm

   CC_OBJ = $(CC) $(CFLAGS) -o $@ -c $<
 #  CC_DLL = $(CC) -shared -Xlinker -Bsymbolic $(CFLAGS) -o $@ $^ -lm -lc 
   CC_DLL = $(CC) -shared -Xlinker -Bsymbolic $(CFLAGS) -o ../bin/$@ $^ -lm -lc 
   
   CC_LIB = ar -r $@ $^
   CC_EXE = $(CC) $^ $(CFLAGS) -o $@ $(LIBS)
   CC_RC = $(CC) $(CFLAGS) -o $@ -c -x c $<
   DLL=so
   TDLL=lib
   LIBEXT=a

   FIND=find
   RM= rm -f
endif


all: clean prebuild libosbdm-jm60.so.1.0

.PHONY: build.txt
build.txt:
	echo OSBDM-JM60 Version $(VERSION) > build.txt

#libosbdm-jm60.so: osbdm_api.o osbdm_base.o osbdm_usb.o osbdm_utils.o
libosbdm-jm60.so.1.0: osbdm_api.o osbdm_base.o osbdm_usb.o osbdm_utils.o
	$(CC_DLL)   -L../libusb_linux -o libosbdm-jm60.so.1.0 -lusb
#	gcc -shared -W1,-soname,libosbdm-jm60.so.1  -Xlinker -Bsymbolic $(CFLAGS) -o libosbdm-jm60.so.1.0 -lm -lc
	mv libosbdm-jm60.so.1.0 ../bin
#	ln -sf ../bin/libosbdm-jm60.so.1.0 ../bin/libosbdm-jm60.so
#	ln -sf ../bin/libosbdm-jm60.so.1.0 ../bin/libosbdm-jm60.so.1
	
osbdm_api.o: osbdm_api.c
	$(CC_OBJ)

osbdm_base.o: osbdm_base.c
	$(CC_OBJ) -I. -I../../osbdmusb_api -I../../../osbdm-common/src

osbdm_usb.o: osbdm_usb.c
	$(CC_OBJ)
	
osbdm_utils.o: osbdm_utils.c
	$(CC_OBJ)
	

prebuild:
	mkdir ../bin
clean:
	rm -rf ../bin *.exp *.dll *.lib *.o 
 

   
