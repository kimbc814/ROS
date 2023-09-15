CFLAGS = -w -Wall -O2
LIBS = -lpthread 'pkg-config --cflags --libs opencv'

WIRINGPI_LIB = -L/path/to/wiringPi/lib

all: test

test: I2C_Thread.c 
	g++ -w -Wall -O2 -o test I2C_Thread.c -lpthread $(pkg-config --cflags --libs opencv) -L/path/to/wiringPi/lib -lwiringPi

	
clean:
	rm -f test
