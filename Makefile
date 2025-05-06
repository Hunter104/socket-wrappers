CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -fPIC
LDFLAGS = 
LIB_NAME = socketwrapper

SRC = base-socket.cpp tcp-socket.cpp udp-socket.cpp
OBJ = $(SRC:.cpp=.o)
HEADERS = base-socket.h tcp-socket.h udp-socket.h

.PHONY: all clean static shared install uninstall

all: static shared

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

static: $(OBJ)
	ar rcs lib$(LIB_NAME).a $(OBJ)
	
shared: $(OBJ)
	$(CXX) -shared -o lib$(LIB_NAME).so $(OBJ) $(LDFLAGS)

install: static shared
	mkdir -p $(DESTDIR)/usr/local/lib
	mkdir -p $(DESTDIR)/usr/local/include/$(LIB_NAME)
	
	cp lib$(LIB_NAME).a $(DESTDIR)/usr/local/lib/
	cp lib$(LIB_NAME).so $(DESTDIR)/usr/local/lib/
	
	cp $(HEADERS) $(DESTDIR)/usr/local/include/$(LIB_NAME)/
	
	ldconfig || true

uninstall:
	rm -f $(DESTDIR)/usr/local/lib/lib$(LIB_NAME).a
	rm -f $(DESTDIR)/usr/local/lib/lib$(LIB_NAME).so
	rm -rf $(DESTDIR)/usr/local/include/$(LIB_NAME)
	ldconfig || true

clean:
	rm -f $(OBJ) lib$(LIB_NAME).a lib$(LIB_NAME).so
