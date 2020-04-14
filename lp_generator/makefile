CC = g++
CFLAGS =  -Wall -g -c -std=c++11

OBJFILES = graph.o parser.o main.o
TARGET = lp_generator

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) -std=c++11 -o $(TARGET) $(OBJFILES)

# lp_translator
main.o: main.cpp
	$(info building main...)
	$(CC) $(CFLAGS) -o $@ $<

# XML decoder / LP generator
parser.o: Parser.cpp
	$(info Building Parser...)
	$(CC) $(CFLAGS) -o $@ $<

# KDG graph core
graph.o: KDG.cpp
	$(info building KDG Graph...)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm *.o
	rm $(TARGET)
