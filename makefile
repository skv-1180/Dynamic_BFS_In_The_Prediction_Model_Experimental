CXX      = g++
CXXFLAGS = -std=c++20 -O2 -Wall
DBGFLAGS = -std=c++20 -O0 -g -fsanitize=address,undefined -Wall

PTH = .
SRC = $(PTH)/src
INC = $(PTH)/include
BIN = $(PTH)/bin

EXEC     = $(BIN)/app
EXEC_DBG = $(BIN)/app_debug

SRCS = \
  $(SRC)/Graph.cpp       \
  $(SRC)/InitGraph.cpp   \
  $(SRC)/BFSState.cpp    \
  $(SRC)/BFSAlgorithms.cpp \
  $(SRC)/Preprocess.cpp  \
  $(SRC)/Incremental.cpp \
  $(SRC)/Decremental.cpp \
  $(SRC)/FullyDynamic.cpp \
  $(SRC)/main.cpp

OBJS     = $(patsubst $(SRC)/%.cpp, $(BIN)/%.o, $(SRCS))
OBJS_DBG = $(patsubst $(SRC)/%.cpp, $(BIN)/%_dbg.o, $(SRCS))

.PHONY: all debug clean run_inc run_dec run_fd verify

all: $(BIN) $(EXEC)

debug: $(BIN) $(EXEC_DBG)

# Release build 
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build 
$(EXEC_DBG): $(OBJS_DBG)
	$(CXX) $(DBGFLAGS) $^ -o $@

$(BIN)/%_dbg.o: $(SRC)/%.cpp
	$(CXX) $(DBGFLAGS) -c $< -o $@

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)

# Convenience run targets 
run_inc: all
	$(EXEC) --mode incremental --verify data/example_incremental.txt

run_dec: all
	$(EXEC) --mode decremental --verify data/example_decremental.txt

run_fd: all
	$(EXEC) --mode fullydynamic --verify data/example_fullydynamic.txt

verify: all
	$(EXEC) --mode incremental  --verify --quiet data/example_incremental.txt
	$(EXEC) --mode decremental  --verify --quiet data/example_decremental.txt
	$(EXEC) --mode fullydynamic --verify --quiet data/example_fullydynamic.txt
	@echo "All verification runs completed."
