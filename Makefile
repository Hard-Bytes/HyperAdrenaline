##################################
# Hyper Adrenaline Main Makefile #
##################################
#---------------------------------#
# USAGE 						  #
#---------------------------------#

	### QUICK GUIDE
	##  First time: 		make libs && make (to compile libraries in your computer then our program)
	##  Subsequent times: 	make			  (to compile our program using already compiled libraries)

	### OPTIONS
	## make				- Compiles the project (not the libraries)
	## make clean		- Removes compilation junk files
	## make cleanall	- Removes compilation junk files AND the main executable compiled
	## make cleanbase	- Removes compilation junk files of most problematic files
	## make cleanai		- Removes compilation junk files of AI-related files
	## make cleannetwork- Removes compilation junk files of networking-related files
	## make cleanfacade - Removes compilation junk files of facade
	## make libs		- Compiles the static libraries, thus generating the .a files
	## make libsclean	- Cleans library compilation junk
	## make libscleanall- Cleans library compilation junk AND library compilation results
	## make server		- Compiles the server
	## make serverclean	- Cleans server compilation junk

#---------------------------#
# MACROS 					#
#---------------------------#
### COMPILE
### Compiles a given source file to an object file
# $(1) : Compiler
# $(2) : Object output file
# $(3) : Source input  file
# $(4) : Additional dependencies
# $(5) : Compiler flags
define COMPILE
$(2): $(3) $(4)
	$(1) $(3) -c -o $(2) $(5)
endef

### C20
### Gets a source file name and gets its object file name (.cpp/.c to .o, and src/ to obj/)
# $(1) : Source file input
define C20
$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(patsubst $(SRC)%,$(OBJ)%,$(1))))
endef

### C2H
### Gets a source file name and gets its header file name (.cpp to .hpp, .c to .h)
# $(1) : Source file input
define C2H
$(patsubst %.c,%.h,$(patsubst %.cpp,%.hpp,$(1)))
endef

#---------------------------#
# VARIABLES 				#
#---------------------------#
## Application Name
APP			:= HyperAdrenaline

## Compiler data
CC			:= g++
C			:= gcc
CCFLAGS		:= -Wall -pedantic -std=c++17 -O3
CFLAGS		:= -Wall -pedantic

## Tools
RM			:= rm -f

## Base directories
SRC			:= src
LIB			:= lib
STATICLIB	:= staticlib
INC			:= include
OBJ			:= obj
## Found files
ALLCPPS		:= $(shell find $(SRC)/ -type f -iname *.cpp)
ALLCS		:= $(shell find $(SRC)/ -type f -iname *.c)
ALLOBJS		:= $(foreach F,$(ALLCPPS) $(ALLCS),$(call C20,$(F)))
## Found directories
SUBDIRS		:= $(shell find $(SRC) -type d)
OBJSUBDIRS	:= $(patsubst $(SRC)%,$(OBJ)%,$(SUBDIRS))

## Libraries and include paths
LIBFMODPTH	:= $(LIB)/fmodapi/api
LIBHYPERPTH	:= $(LIB)/hyperengine
LIBOPENGLPTH:= $(LIB)/opengl
LIBBULLETPTH:= $(LIB)/bullet
LIBBOOSTPTH	:= $(LIB)/boost

FMODLIBS	:= $(LIBFMODPTH)/core/lib/x86_64/libfmod.so $(LIBFMODPTH)/studio/lib/x86_64/libfmodstudio.so
HYPERLIBS	:= $(LIBHYPERPTH)/libassimp.so.5 $(LIBHYPERPTH)/libSOIL.so.1
OPENGLLIBS	:= $(LIBOPENGLPTH)/libglfw.so.3 $(LIBOPENGLPTH)/libGLU.so.1 $(LIBOPENGLPTH)/libglut.so.3 $(LIBOPENGLPTH)/libGL.so.1 $(LIBOPENGLPTH)/libGLEW.so.2.2 $(LIBOPENGLPTH)/libGLdispatch.so.0 $(LIBOPENGLPTH)/libGLX.so.0
BULLETLIBS	:= $(LIBBULLETPTH)/libBulletCollision.so.3.08 $(LIBBULLETPTH)/libBulletSoftBody.so.3.08 $(LIBBULLETPTH)/libBulletDynamics.so.3.08 $(LIBBULLETPTH)/libBulletWorldImporter.so.3.08 $(LIBBULLETPTH)/libLinearMath.so.3.08 $(LIBBULLETPTH)/libBulletFileLoader.so.3.08
BOOSTLIBS	:= $(LIBBOOSTPTH)/libboost_random.so.1.75.0 $(LIBBOOSTPTH)/libboost_system.so.1.75.0 $(LIBBOOSTPTH)/libboost_thread.so.1.75.0
ALLSOLIBS	:= $(FMODLIBS) $(HYPERLIBS) $(OPENGLLIBS) $(BULLETLIBS) $(BOOSTLIBS)
HYPERSTATIC	:= $(STATICLIB)/hyperengine/libhyperengine.a
SHAREDLINK	:= -lX11 -lm -lpthread
LIBS		:= $(SHAREDLINK) $(HYPERSTATIC) $(ALLSOLIBS)
SHPATHS 	:= -L$(LIB)/fmodapi/api/studio/lib/x86_64 -L$(LIB)/fmodapi/api/core/lib/x86_64 -L$(LIBHYPERPTH) -L$(LIBOPENGLPTH) -L$(LIBBULLETPTH) -L$(LIBBOOSTPTH)
RHPATHS 	:= -rpath=$(LIBFMODPTH)/core/lib/x86_64,-rpath=$(LIBFMODPTH)/studio/lib/x86_64,-rpath=$(LIBHYPERPTH),-rpath=$(LIBOPENGLPTH),-rpath=$(LIBBULLETPTH),-rpath=$(LIBBOOSTPTH)
SHARED_PATH := $(SHPATHS) -Wl,$(RHPATHS)
INCDIRS		:= -I/usr/include/GL/ -I/usr/include/GLFW/ -I$(INC)/imgui/ -I$(INC)/hyperengine/ -I$(INC)/rapidJson/include -I$(INC)/fmodapi/api/core/inc/ -I$(INC)/fmodapi/api/studio/inc/ -I$(INC)/websocketpp -I/usr/include/bullet/#-I$(INC)/bullet/

## Tools
MKDIR		:= mkdir -p

## Optional flags
ifdef ASAN
	CCFLAGS += -fsanitize=address
endif

#---------------------------------#
# DEPENDECIES AND GENERATED RULES #
#---------------------------------#
.PHONY: info

$(APP): $(OBJSUBDIRS) $(ALLOBJS)
	$(CC) $(SHARED_PATH) $(ALLOBJS) -o $(APP) $(LIBS) $(CCFLAGS)

# Compile all .cpp and .c files into .o files
$(foreach F, $(ALLCPPS),$(eval $(call COMPILE,$(CC),$(call C20,$(F)),$(F),$(call C2H,$(F)),$(CCFLAGS) $(INCDIRS))))
$(foreach F, $(ALLCS),$(eval $(call COMPILE,$(C),$(call C20,$(F)),$(F),$(call C2H,$(F)),$(CFLAGS) $(INCDIRS))))

info:
	$(info $(SUBDIRS))
	$(info $(OBJSUBDIRS))
	$(info $(ALLCPPS))
	$(info $(ALLCS))
	$(info $(ALLOBJS))

$(OBJSUBDIRS):
	$(MKDIR) $(OBJSUBDIRS)

clean:
	$(RM) -r "./$(OBJ)"
cleanall: clean
	$(RM) -r "./$(APP)"
cleanbase:
	$(RM) -r "./$(OBJ)/main.o"
	$(RM) -r "./$(OBJ)/game.o"
	$(RM) -r "./$(OBJ)/system"
	$(RM) -r "./$(OBJ)/components"
	$(RM) -r "./$(OBJ)/manager"
	$(RM) -r "./$(OBJ)/ecs"
	$(RM) -r "./$(OBJ)/states"
	$(RM) -r "./$(OBJ)/context"
cleanai:
	$(RM) -r "./$(OBJ)/behaviours"
	$(RM) -r "./$(OBJ)/pathfinding"
cleannetwork:
	$(RM) -r "./$(OBJ)/network"
cleanfacade:
	$(RM) -r "./$(OBJ)/facade"

haserver:
	$(MAKE) -C server
serverclean:
	$(MAKE) -C server clean

help:
	$(info ### QUICK GUIDE)
	$(info |   make         To compile)
	$(info |   make -j n    To use multithread compilation add -j n (where n is the number of threads), for example: make -j 6)
	$(info |   make ASAN=1  To use Address Sanitizer add the ASAN flag, for example: make ASAN=1)
	$(info |)
	$(info |   ex: make ASAN=1 -j 6    will compile with 6 threads and using Address Sanitizer)
	$(info )
	$(info ### OPTIONS)
	$(info |   make               - Compiles the project)
	$(info |   make clean         - Removes compilation junk files)
	$(info |   make cleanall      - Removes compilation junk files AND the main executable)
	$(info |   make cleanbase     - Removes compilation junk files of system, components, manager, states, game and main)
	$(info |   make cleanai       - Removes compilation junk files of AI-related files (behaviours and pathfinding))
	$(info |   make cleannetwork  - Removes compilation junk files of networking-related files (network))
	$(info |   make cleanfacade   - Removes compilation junk files of facade)
	$(info |   )
	$(info |   make haserver      - Compiles the server)
	$(info |   make serverclean   - Cleans server compilation junk)
	$(info |   )
