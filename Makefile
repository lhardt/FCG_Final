# Commands 
RM 		:= -rm
CC 		:= /C/msys64/mingw64/bin/g++.exe

# Folders
SRCDIR 	:= src
OBJDIR 	:= obj
LIBDIR  := lib
BINDIR 	:= bin

# Files
#  -lGL is for Unix, -lopengl32 for Windows 
CSRC 	:= $(wildcard $(SRCDIR)/*.c) 
CPPSRC  := $(wildcard $(SRCDIR)/*.cpp) 
MAIN 	:= $(wildcard $(SRCDIR)/main.*)
LIB     := -lopengl32 -lm -lpthread -lglfw3 # $(wildcard $(LIBDIR)/*)
COBJ     := $(CSRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CPPOBJ   := $(CPPSRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.opp)

TARGET  := $(BINDIR)/main.exe

# Flags
CXXFLAGS := -Wall -O2 -std=c++17 -Iinclude 
	
all: $(TARGET)
$(TARGET) : $(COBJ) $(CPPOBJ)
	$(CC) -o $(TARGET) $(COBJ) $(CPPOBJ) $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.opp: $(SRCDIR)/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean: 
	$(RM) $(COBJ) $(CPPOBJ) $(TARGET)

#	$(CC) -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/Linux/main.exe src/tiny_obj_loader.cpp src/main.cpp src/glad.c src/textrendering.cpp  -lopengl32 -lm -lpthread -lglfw3

.PHONY: clean run

run: ./bin/main.exe
	cd bin && ./main.exe
