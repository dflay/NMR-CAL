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
DEPS2=$(INC_DIR)/SG382Interface.hh $(INC_DIR)/SG382.hh $(DRIVER_INC_DIR)/CommDriver.hh $(DRIVER_INC_DIR)/Device.hh
# CPPFLAGS   = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wl,-rpath,$(LIB_DIR)
CPPFLAGS   = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wall
LIBFLAGS   = -L$(LIB_DIR) -lSG382Interface -lSG382
DRIVER_LIB_FLAGS = -L$(DRIVER_LIB_DIR) -lCommDriver 

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libSG382Interface.so $(LIB_DIR)/libSG382.so

$(LIB_DIR)/libSG382.so: $(BUILD_DIR)/SG382.o $(BUILD_DIR)/SG382Interface.o $(DRIVER_BUILD_DIR)/CommDriver.o $(DRIVER_BUILD_DIR)/Device.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/SG382.o: $(SRC_DIR)/SG382.cxx $(DEPS2)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)

$(LIB_DIR)/libSG382Interface.so: $(BUILD_DIR)/SG382Interface.o $(DRIVER_BUILD_DIR)/CommDriver.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/SG382Interface.o: $(SRC_DIR)/SG382Interface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)   

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libSG382Interface.so $(LIB_DIR)/libSG382.so
	$(CXX) -o $@ $< $(CPPFLAGS)  $(LIBFLAGS) $(ROOTLIBS) 

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS2)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS) $(ROOTFLAGS) 

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
