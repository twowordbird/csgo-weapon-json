BIN_DIR = bin
SRC_DIR = src

HL2SDK_DIR = ../hl2sdk-ob-valve-4f3ac1d94787
CSGO_DIR = /home/chris/Steam/games/csgo_ds

CXXFLAGS += -g -m32 -msse3 -DPLATFORM_POSIX -D_LINUX -DCOMPILER_GCC -DCSGO_DIR=\"$(CSGO_DIR)\" \
	-DTEST_DIR=\"$(SRC_DIR)/tests\" -I$(HL2SDK_DIR)/public -I$(SRC_DIR)
LDFLAGS += -L$(CSGO_DIR)/bin -L$(HL2SDK_DIR)/lib/linux \
	$(HL2SDK_DIR)/lib/linux/tier1_i486.a \
	-ltier0_srv -lvstdlib \
	-Wl,-rpath=$(CSGO_DIR)/bin -Wl,-rpath=$(HL2SDK_DIR)/lib/linux

BIN = weaponjson runtests

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst %.cpp,%.o,$(patsubst $(SRC_DIR)/%,$(BIN_DIR)/%,$(SRC)))

TEST_SRC = $(wildcard $(SRC_DIR)/tests/*.cpp)
TEST_OBJ = $(patsubst %.cpp,%.o,$(patsubst $(SRC_DIR)/%,$(BIN_DIR)/%,$(TEST_SRC)))

all: $(BIN) test

weaponjson: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

runtests: $(filter-out bin/main.o,$(OBJ)) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lcppunit

test: runtests
	./runtests

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(TEST_OBJ)
