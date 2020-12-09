TARGET = server

all:$(TARGET)

CC = gcc
CXX = g++
STRIP = strip
ARMCC = arm-linux-gcc
ARMSTRIP = arm-linux-strip

#where to install
INSTDIR = bin

#where are include files kept
INCLUDE = 

#CFLAGS  = -g -Wall –lstdc++
CFLAGS = -g -Wall

RANLIB = ranlib

SOURCES = q_server.c lstLib.c

C_SRCS = $(filter %.c, $(SOURCES))

C_OBJS = $(C_SRCS:%.c=release/%.o)

$(C_OBJS):release/%.o:%.c
	@mkdir -p release
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@
	
$(TARGET):$(C_OBJS)
	$(CC) $(CFLAGS) $^ -lpthread -o $@
	#mkdir -p $(INSTDIR)/debug
	#cp $@ $(INSTDIR)/debug
	#mkdir -p $(INSTDIR)/release
	#$(STRIP) $@
	#cp $@ $(INSTDIR)/release
	
clean:
	-rm -rf release
	-rm -rf $(TARGET)
	