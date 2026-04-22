CXX      = g++
CXXFLAGS = -std=c++20 -O2 -Wall
DBGFLAGS = -std=c++20 -O0 -g -fsanitize=address,undefined -Wall

PTH   = .
SRC   = $(PTH)/src
INC   = $(PTH)/include
TESTS = $(PTH)/tests
BIN   = $(PTH)/bin
OBJ   = $(BIN)/obj

EXEC       = $(BIN)/app
EXEC_DBG   = $(BIN)/app_debug
EXEC_BENCH = $(BIN)/benchmark

SRCS = \
  $(SRC)/Graph.cpp \
  $(SRC)/InitGraph.cpp \
  $(SRC)/BFSState.cpp \
  $(SRC)/utils.cpp \
  $(SRC)/Preprocess.cpp \
  $(SRC)/Incremental.cpp \
  $(SRC)/Decremental.cpp \
  $(SRC)/FullyDynamic.cpp \
  $(SRC)/main.cpp

BENCH_SRCS = \
  $(SRC)/Graph.cpp \
  $(SRC)/InitGraph.cpp \
  $(SRC)/BFSState.cpp \
  $(SRC)/utils.cpp \
  $(SRC)/Preprocess.cpp \
  $(SRC)/Incremental.cpp \
  $(SRC)/Decremental.cpp \
  $(SRC)/FullyDynamic.cpp \
  $(TESTS)/helper.cpp \
  $(TESTS)/Metrics.cpp \
  $(TESTS)/ClassicalBFS.cpp \
  $(TESTS)/benchmark.cpp

OBJS     = $(patsubst $(SRC)/%.cpp,$(OBJ)/src/%.o,$(SRCS))
OBJS_DBG = $(patsubst $(SRC)/%.cpp,$(OBJ)/src/%_dbg.o,$(SRCS))

BENCH_OBJS = \
  $(patsubst $(SRC)/%.cpp,$(OBJ)/src/%.o,$(filter $(SRC)/%.cpp,$(BENCH_SRCS))) \
  $(patsubst $(TESTS)/%.cpp,$(OBJ)/tests/%.o,$(filter $(TESTS)/%.cpp,$(BENCH_SRCS)))

.PHONY: all debug bench clean run_inc run_dec run_fd verify

all: $(EXEC)

debug: $(EXEC_DBG)

bench: $(EXEC_BENCH)

# Release app
$(EXEC): $(OBJS) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Debug app
$(EXEC_DBG): $(OBJS_DBG) | $(BIN)
	$(CXX) $(DBGFLAGS) $^ -o $@

# Benchmark
$(EXEC_BENCH): $(BENCH_OBJS) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile src release objects
$(OBJ)/src/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC) -I$(TESTS) -c $< -o $@

# Compile src debug objects
$(OBJ)/src/%_dbg.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(DBGFLAGS) -I$(INC) -I$(TESTS) -c $< -o $@

# Compile tests release objects
$(OBJ)/tests/%.o: $(TESTS)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC) -I$(TESTS) -c $< -o $@

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