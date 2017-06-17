CXX=g++
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib
BUILD_DIR = ./build
BIN_DIR = ./bin

# ROOT stuff
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS  = $(shell root-config --libs)

# Locations and default files
PREFIX=/usr/local
LIBINSTALLDIR=$(PREFIX)/lib
INCINSTALLDIR=$(PREFIX)/include/g2field

DEPS=$(INC_DIR)/SG382Interface.hh
CPPFLAGS = -I$(INC_DIR) -Wl,-rpath,$(LIB_DIR)
LIBFLAGS = -L$(LIB_DIR) -lsg382interface  

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libsg382interface.so

$(BUILD_DIR)/SG382Interface.o: $(SRC_DIR)/SG382Interface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS) $(DRIVER_LIB_FLAGS) 

$(LIB_DIR)/libsg382interface.so: $(BUILD_DIR)/SG382Interface.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS) $(ROOTFLAGS) 

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libsg382interface.so
	$(CXX) -o $@ $< $(CPPFLAGS)  $(LIBFLAGS) $(ROOTLIBS) 

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
