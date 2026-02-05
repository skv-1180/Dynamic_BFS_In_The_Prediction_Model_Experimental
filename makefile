CC = g++ -std=c++20
CFLAGS = -O3 -c
PTH = .
SRC = $(PTH)/src
INC = $(PTH)/include
BIN = $(PTH)/bin
TST = $(PTH)/tests

EXEC = $(BIN)/app

.PHONY: clean all

all: $(EXEC)

$(BIN):
	mkdir -p $(BIN)

$(EXEC): \
	$(BIN)/Graph.o \
	$(BIN)/InitGraph.o \
	$(BIN)/Preprocess.o \
	$(BIN)/BFSSnapshotStore.o \
	$(BIN)/main.o
	$(CC) $^ -o $@

$(BIN)/Graph.o: $(SRC)/Graph.cpp $(INC)/Graph.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/InitGraph.o: $(SRC)/InitGraph.cpp $(INC)/InitGraph.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/Preprocess.o: $(SRC)/Preprocess.cpp $(INC)/Preprocess.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/BFSSnapshotStore.o: $(SRC)/BFSSnapshotStore.cpp $(INC)/BFSSnapshotStore.h $(INC)/Graph.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/main.o: $(SRC)/main.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN)/*
