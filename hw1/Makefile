CC=g++ -std=c++17
CFLAGS=-g3 -I$(SYSTEMC_HOME)/include
LFLAGS=-L$(SYSTEMC_HOME)/lib-linux64 -Wl,-rpath=$(SYSTEMC_HOME)/lib-linux64 -lsystemc

DEPS=hw1.h bmp.h
OBJS=hw1.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(IFLAGS)

hw1: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) hw1 before.bmp after.bmp
