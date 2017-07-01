CXX=g++
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib
BUILD_DIR = ./build
BIN_DIR = ./bin
DRIVER_INC_DIR   = ../comm-driver/include
DRIVER_BUILD_DIR = ../comm-driver/build
DRIVER_LIB_DIR   = ../comm-driver/lib

# Locations and default files
PREFIX=/usr/local
LIBINSTALLDIR=$(PREFIX)/lib
INCINSTALLDIR=$(PREFIX)/include/g2field

DEPS=$(INC_DIR)/KeithleyInterface.hh 
DEPS2=$(INC_DIR)/KeithleyInterface.hh $(INC_DIR)/KeithleyClass.hh $(DRIVER_INC_DIR)/Device.hh 
# CFLAGS = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wl,-rpath,$(LIB_DIR)
CFLAGS = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wall
LIBFLAGS = -L$(LIB_DIR) -lkeithleyinterface -lkeithleyclass 
DRIVER_LIB_FLAGS = -L$(DRIVER_LIB_DIR) -lCommDriver

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libkeithleyinterface.so $(LIB_DIR)/libkeithleyclass.so

$(LIB_DIR)/libkeithleyinterface.so: $(BUILD_DIR)/KeithleyInterface.o $(DRIVER_BUILD_DIR)/CommDriver.o 
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/KeithleyInterface.o: $(SRC_DIR)/KeithleyInterface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(LIB_DIR)/libkeithleyclass.so: $(BUILD_DIR)/KeithleyClass.o $(DRIVER_BUILD_DIR)/CommDriver.o $(DRIVER_BUILD_DIR)/Device.o 
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/KeithleyClass.o: $(SRC_DIR)/KeithleyClass.cxx $(DEPS2) 
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS) `root-config --cflags`

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libkeithleyinterface.so $(LIB_DIR)/libkeithleyclass.so
	$(CXX) -o $@ $< $(CFLAGS)  $(LIBFLAGS) `root-config --libs`

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
