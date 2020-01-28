HOST ?= OSX
SRC = main.cpp 
INCLUDE = src
BIN = bin
SRCDIR = src
OBJDIR = bin
CPPSRC = $(SRC:%.cpp=$(SRCDIR)/%.cpp)
OBJ = $(SRC:%.cpp=$(OBJDIR)/%.o)
GENDEPFLAGS =
VPATH = src 
CXX = g++
STRIP = strip
ifeq ($(HOST),win32)
CPPFLAGS = -g -o2 -Wall -I$(INCLUDE)  -finput-charset=utf_8 -fexec-charset=gbk
PREFIX ?= /c/windows
SUDO :=
INSTALL = cp
LDFLAGS =  -static 
BINFILE=hex2rom.exe
else 
CPPFLAGS = -g -o2 -Wall -I$(INCLUDE)
SUDO ?= sudo
INSTALL =install -m 644
PREFIX ? = /usr/bin
LDFLAGS =   
BINFILE=hex2rom
LDFLAGS =
endif
TARGET = $(BIN)/$(BINFILE)
all:build
install: strip
	$(SUDO) $(INSTALL) $(TARGET) $(PREFIX)
uninstall:
	$(SUDO) rm -f $(PREFIX)/$(BINFILE)
build : $(TARGET)
$(TARGET): $(OBJ)
	$(CXX)  $(LDFLAGS)  $^ -o $@ 
$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	$(CXX)  $(CPPFLAGS) -c $< -o $@   
	 
$(OBJDIR)/main.o: $(SRCDIR)/main.cpp 

strip:$(TARGET)
	$(STRIP) $<
clean:
	rm -rf  $(BIN)
# Create object files directory
$(shell mkdir  $(OBJDIR) $(BIN) 2>/dev/null)
.PHONY : all clean build strip install uninstall 
	

