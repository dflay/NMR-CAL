CXX=g++
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib
BUILD_DIR = ./build
BIN_DIR = ./bin
DRIVER_INC_DIR   = ../comm-driver/include
DRIVER_BUILD_DIR = ../comm-driver/build
DRIVER_LIB_DIR   = ../comm-driver/lib

# ROOT stuff
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS  = $(shell root-config --libs)

# Locations and default files
PREFIX=/usr/local
LIBINSTALLDIR=$(PREFIX)/lib
INCINSTALLDIR=$(PREFIX)/include/g2field

DEPS=$(INC_DIR)/SG382Interface.hh $(DRIVER_INC_DIR)/CommDriver.hh
# CPPFLAGS   = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wl,-rpath,$(LIB_DIR)
CPPFLAGS   = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wall
LIBFLAGS   = -L$(LIB_DIR) -lsg382interface 
DRIVER_LIB_FLAGS = -L$(DRIVER_LIB_DIR) -lCommDriver 

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libsg382interface.so

$(LIB_DIR)/libsg382interface.so: $(BUILD_DIR)/SG382Interface.o $(DRIVER_BUILD_DIR)/CommDriver.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/SG382Interface.o: $(SRC_DIR)/SG382Interface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)   

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libsg382interface.so
	$(CXX) -o $@ $< $(CPPFLAGS)  $(LIBFLAGS) $(ROOTLIBS) 

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS) $(ROOTFLAGS) 

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
