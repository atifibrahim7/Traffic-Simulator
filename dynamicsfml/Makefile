CXX = g++
CXXFLAGS = -I/usr/include/SFML -I/usr/include
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -ltinyxml2

all: myProgram

myProgram: main.o
	$(CXX) main.o -o myProgram $(LDFLAGS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o myProgram
