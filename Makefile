CXX = g++
CXXFLAGS = -std=c++14 -O3 -march=native

PROGRAM_NAME = DonutAI

BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj

HEADER_FILES = $(wildcard $(SRC_DIR)/*.h)
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))
EXECUTABLE := $(BIN_DIR)/$(PROGRAM_NAME)

HOST_JAR = ./ConnectK_1.8.jar

.PHONY: default
default: all

.PHONY: all
all: $(EXECUTABLE)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) .depend

.PHONY: depend
depend: .depend

.depend: $(CPP_FILES)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^ >> ./.depend;

include .depend

$(EXECUTABLE): $(OBJ_FILES) $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR) $(HEADER_FILES)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir $@

$(BIN_DIR):
	mkdir $@

.PHONY: run
run: $(EXECUTABLE)
	java -jar $(HOST_JAR) -k:4 -g:1 cpp:$(EXECUTABLE)