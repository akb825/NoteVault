DCOMP = gdc
DFLAGS = -O3 -c
LDFLAGS = -O3
SOURCE_DIR = ../../src/desktop
OBJ_DIR = objs
BIN_DIR = bin
EXECUTABLE = NoteVault

SOURCES = main.d

SOURCE_PATHS = $(patsubst %,$(SOURCE_DIR)/%,$(SOURCES))
OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(SOURCES:.d=.o))
EXECUTABLE_PATH = $(BIN_DIR)/$(EXECUTABLE)

all: $(SOURCE_PATHS) $(EXECUTABLE_PATH) 

$(EXECUTABLE_PATH): $(OBJ_DIR) $(BIN_DIR) $(OBJECTS)
	$(DCOMP) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.d
	$(DCOMP) $(DFLAGS) $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -r $(OBJ_DIR) $(BIN_DIR)
