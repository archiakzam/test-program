CXX = g++
CXXFLAGS = -std=c++17 -pthread -O2 -Wall -Iinclude -I.
TARGET = monitor
SOURCES = main.cpp \
          src/Application.cpp \
          src/CPUCollector.cpp \
          src/DiskCollector.cpp \
          src/HttpServer.cpp \
          src/ProcessCollector.cpp \
          src/RAMCollector.cpp \
          src/SystemStats.cpp \
          src/TempCollector.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run