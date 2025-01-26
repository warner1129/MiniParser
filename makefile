CXX = g++
CXXFLAGS = -std=c++20 -Iinclude -fsanitize=address,undefined -g
TARGET = libminiparser.a
SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, obj/%.o, $(SRCS))

all: libminiparser.a testing

libminiparser.a: $(OBJS)
	ar rcs $@ $^

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/test: tests/test.cpp $(TARGET)
	$(CXX) $(CXXFLAGS) tests/test.cpp $(TARGET) -o tests/test

testing: obj/test
	./tests/test

demo: $(TARGET)
	$(CXX) $(CXXFLAGS) demo/demo1.cpp $(TARGET) -o demo/demo1
	$(CXX) $(CXXFLAGS) demo/demo2.cpp $(TARGET) -o demo/demo2
	$(CXX) $(CXXFLAGS) demo/demo3.cpp $(TARGET) -o demo/demo3
	$(CXX) $(CXXFLAGS) demo/demo4.cpp $(TARGET) -o demo/demo4
	$(CXX) $(CXXFLAGS) demo/demo5.cpp $(TARGET) -o demo/demo5

clean:
	rm $(OBJS) $(TARGET) obj/test
