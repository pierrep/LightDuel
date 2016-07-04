PROG = PongLinux
#PROG = udev_simple

TARGET = $(PROG)
#CC = clang
CC = gcc -O3 -Wall
STRIP = strip
#CFLAGS = -Wall -O2 -DOS_$(OS)
#CFLAGS = -pthread -lm -g 
#-mcpu= -mtune=cortex-a7 cortex-a15 -mfpu=neon-vfpv4 -mfloat-abi=hard -ffast-math
CFLAGS = -pthread -O3 -Wall -g -mtune=cortex-a7 -mcpu=cortex-a15 -mfpu=neon-vfpv4 -mfloat-abi=hard -ffast-math
#LIBS = -v -lusb -ludev -lliblo
LIBS = -lusb -ludev -llo
#LIBS = -lusb -ludev

OBJS = $(PROG).o

all: $(TARGET)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)
	$(STRIP) $(PROG)

clean:
	rm -f *.o $(PROG)
	rm -rf tmp
