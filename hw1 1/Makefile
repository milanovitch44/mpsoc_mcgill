CC=g++ -std=c++17
CFLAGS=-g3
LFLAGS=-lsystemc
DEPS=hw1.h bmp.h
OBJS=hw1.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(IFLAGS)

hw1: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) hw1 before.bmp after.bmp
