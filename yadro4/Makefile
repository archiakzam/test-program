COMP = g++
COMPFLAGS = -std=c++17 -Wall -pthread
LDFLAGS = -pthread

ENABLE_HTTP ?= 1

ifeq ($(ENABLE_HTTP),1)
    COMPFLAGS += -DENABLE_HTTP
    HTTP_SRCS = src/HttpService.cpp
    HTTP_OBJS = $(HTTP_SRCS:.cpp=.o)
else
    HTTP_SRCS =
    HTTP_OBJS =
endif

SRCS = src/main.cpp src/Scaner.cpp src/JsonBuilder.cpp $(HTTP_SRCS)
OBJS = $(SRCS:.cpp=.o)
TARGET = media_scanner


.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJS)
	$(COMP) $(LDFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(COMP) $(COMPFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

