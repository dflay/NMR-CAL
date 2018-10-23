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

DEPS_BASE=$(INC_DIR)/SISBase.hh $(DRIVER_INC_DIR)/CommDriver.hh $(DRIVER_INC_DIR)/Device.hh
DEPS_3302=$(INC_DIR)/SIS3302.hh $(DEPS_BASE) 
DEPS_3316=$(INC_DIR)/SIS3316.hh $(DEPS_BASE) 
DEPS_ALL=$(INC_DIR)/SIS3316.hh $(INC_DIR)/SIS3302.hh $(DEPS_BASE) 
# CPPFLAGS   = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wl,-rpath,$(LIB_DIR)
CPPFLAGS   = -I$(INC_DIR) -I$(DRIVER_INC_DIR) -Wall
LIBFLAGS   = -L$(LIB_DIR) -lSIS3302
DRIVER_LIB_FLAGS = -L$(DRIVER_LIB_DIR) -lCommDriver 

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libSISBase.so $(LIB_DIR)/libSIS3302.so 
# all: $(BIN_DIR)/TestComm $(LIB_DIR)/libSISInterface.so

$(LIB_DIR)/libSISBase.so: $(BUILD_DIR)/SISBase.o $(DRIVER_BUILD_DIR)/CommDriver.o $(DRIVER_BUILD_DIR)/Device.o \
	$(DRIVER_BUILD_DIR)/vme_calls.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/SISBase.o: $(SRC_DIR)/SISBase.cxx $(DEPS_BASE)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)

$(LIB_DIR)/libSIS3302.so: $(BUILD_DIR)/SIS3302.o $(BUILD_DIR)/SISBase.o $(DRIVER_BUILD_DIR)/CommDriver.o \
	$(DRIVER_BUILD_DIR)/Device.o $(DRIVER_BUILD_DIR)/vme_calls.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/SIS3302.o: $(SRC_DIR)/SIS3302.cxx $(DEPS_3302)
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)   

# $(LIB_DIR)/libSIS3316.so: $(BUILD_DIR)/SIS3316.o $(BUILD_DIR)/SISBase.o $(DRIVER_BUILD_DIR)/CommDriver.o $(DRIVER_BUILD_DIR)/Device.o
# 	$(CXX) -shared -fPIC -o $@ $^ 
# 
# $(BUILD_DIR)/SIS3316.o: $(SRC_DIR)/SIS3316.cxx $(DEPS_3316)
# 	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libSIS3302.so
	$(CXX) -o $@ $< $(CPPFLAGS) $(LIBFLAGS) $(ROOTLIBS) 

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS_3302) 
	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS) $(ROOTFLAGS)

# $(LIB_DIR)/libSISInterface.so: $(BUILD_DIR)/SISInterface.o $(DRIVER_BUILD_DIR)/CommDriver.o
# 	$(CXX) -shared -fPIC -o $@ $^ 

# $(BUILD_DIR)/SISInterface.o: $(SRC_DIR)/SISInterface.cxx $(DEPS)
# 	$(CXX) -o $@ -fPIC -c $< $(CPPFLAGS)   

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
