#Common setup
.SECONDEXPANSION:
SOURCE_DIR = ../../../src/desktop
OBJ_DIR = objs
EXECUTABLE = NoteVault

#Source files
DSOURCES = main.d \
	ui/MainWindow.d

CPPSOURCES = ui/MainWindow.cpp

#D options
DCOMP = gdc
DFLAGS = -O3 -c -Wall -I$(SOURCE_DIR)
DLDFLAGS = -O3 -lstdc++

#C++ options
CPPCOMP = g++
CPPFLAGS = -O3 -c -Wall -std=c++11 -I$(SOURCE_DIR)

#Inlcude directories
ifndef WXCXXFLAGS
	WXCXXFLAGS = $(shell wx-config --cxxflags)
endif
CPPFLAGS += $(WXCXXFLAGS)

#Libraries
ifndef WXLIBS
	WXLIBS =$(shell wx-config --libs) 
endif
DLDFLAGS += $(WXLIBS)

DSOURCE_PATHS = $(patsubst %,$(SOURCE_DIR)/%,$(DSOURCES))
DOBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(subst /,_,$(DSOURCES:.d=.d.o)))
CPPSOURCE_PATHS = $(patsubst %,$(SOURCE_DIR)/%,$(CPPSOURCES))
CPPOBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(subst /,_,$(CPPSOURCES:.cpp=.cpp.o)))

all: $(DSOURCE_PATHS) $(CPPSOURCE_PATHS) $(EXECUTABLE) 

$(EXECUTABLE): $(OBJ_DIR) $(DOBJECTS) $(CPPOBJECTS)
	$(DCOMP) $(DLDFLAGS) $(DOBJECTS) $(CPPOBJECTS) -o $@

$(OBJ_DIR)/%.d.o: $(SOURCE_DIR)/$$(subst _,/,%.d)
	$(DCOMP) $(DFLAGS) $< -o $@

$(OBJ_DIR)/%.cpp.o: $(SOURCE_DIR)/$$(subst _,/,%.cpp)
	$(CPPCOMP) $(CPPFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)
