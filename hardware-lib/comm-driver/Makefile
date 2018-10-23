CXX=g++
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib
BUILD_DIR = ./build
BIN_DIR = ./bin

# Locations and default files
PREFIX=/usr/local
LIBINSTALLDIR=$(PREFIX)/lib
INCINSTALLDIR=$(PREFIX)/include/g2field

DEPS_VME=$(INC_DIR)/sis3100_vme_calls.h $(INC_DIR)/sis1100_var.h 
DEPS=$(INC_DIR)/CommDriver.hh $(INC_DIR)/Device.hh $(DEPS_VME) 
CFLAGS = -I$(INC_DIR) -Wall 
# CFLAGS = -I$(INC_DIR) -Wl,-rpath,$(LIB_DIR)
LIBFLAGS = -L$(LIB_DIR) -lCommDriver

# all: $(LIB_DIR)/libCommDriver.so
# all: $(BUILD_DIR)/CommDriver.so $(BUILD_DIR)/Device.o $(BUILD_DIR)/vme_calls.o 
all: $(LIB_DIR)/libCommDriver.so $(BUILD_DIR)/Device.o 

$(BUILD_DIR)/CommDriver.o: $(SRC_DIR)/CommDriver.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/vme_calls.o: $(SRC_DIR)/sis3100_vme_calls.c $(DEPS_VME)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/Device.o: $(SRC_DIR)/Device.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(LIB_DIR)/libCommDriver.so: $(BUILD_DIR)/CommDriver.o $(BUILD_DIR)/vme_calls.o 
	$(CXX) -shared -fPIC -o $@ $^ 

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* 

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
