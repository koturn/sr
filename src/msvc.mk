### This Makefile was written for nmake. ###
CC         = cl
CPP        = cl
INCS       = /Ilibrary /Ilibrary\getopt
CFLAGS     = /O2 /Wall $(INCS)
CPPFLAGS   = /O2 /Wall $(INCS)
LDLIBS     = shlwapi.lib
LDFLAGS    = /O2
RM         = del /F

GETOPT_DIR = library\getopt
TARGET     = sr.exe
OBJ1       = $(TARGET:.exe=.obj)
OBJ2       = directory.obj
OBJ3       = $(GETOPT_DIR)\getopt.obj
OBJ4       = $(GETOPT_DIR)\getopt_long.obj
SRC1       = $(OBJ1:.obj=.cpp)
SRC2       = $(OBJ2:.obj=.cpp)


all : $(TARGET)

$(TARGET) : $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)
	$(CPP) $(LDLIBS) $(LDFLAGS) $**

$(OBJ1) : $(SRC1)

$(OBJ2) : $(SRC2)

$(OBJ3) $(OBJ4) :
	cd $(GETOPT_DIR)  &&  $(MAKE) /f msvc.mk  &&  cd ..\..


allclean :
	$(RM) $(TARGET) $(OBJ1) $(OBJ2) *.stackdump *~
	cd $(GETOPT_DIR)  &&  $(MAKE) /f msvc.mk $@  &&  cd ..\..
clean :
	$(RM) $(TARGET) $(OBJ1) $(OBJ2) $(TARGET:.exe=.exp) $(TARGET:.exe=.lib)
	cd $(GETOPT_DIR)  &&  $(MAKE) /f msvc.mk $@  &&  cd ..\..
objclean :
	$(RM) $(OBJ1) $(OBJ2)
	cd $(GETOPT_DIR)  &&  $(MAKE) /f msvc.mk $@  &&  cd ..\..
