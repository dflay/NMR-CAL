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

DEPS=$(INC_DIR)/FPGAInterface.hh 
DEPS2=$(INC_DIR)/FPGAInterface.hh $(INC_DIR)/fpga_pulse_sequence.hh $(INC_DIR)/fpga_addresses.hh $(DRIVER_INC_DIR)/Device.hh 
# CFLAGS = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wl,-rpath,$(LIB_DIR)
CFLAGS = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wall
LIBFLAGS = -L$(LIB_DIR) -lfpgainterface 
DRIVER_LIB_FLAGS = -L$(DRIVER_LIB_DIR) -lCommDriver

# all: $(BIN_DIR)/TestComm $(LIB_DIR)/libfpgainterface.so $(LIB_DIR)/libfpgaclass.so
all: $(LIB_DIR)/libfpgainterface.so 

$(LIB_DIR)/libfpgainterface.so: $(BUILD_DIR)/FPGAInterface.o $(DRIVER_BUILD_DIR)/CommDriver.o 
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/FPGAInterface.o: $(SRC_DIR)/FPGAInterface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

# $(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
# 	$(CXX) -o $@ -fPIC -c $< $(CFLAGS) `root-config --cflags`
# 
# $(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libfpgainterface.so 
# 	$(CXX) -o $@ $< $(CFLAGS)  $(LIBFLAGS) `root-config --libs`

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
