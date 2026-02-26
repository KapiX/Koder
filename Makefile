## Haiku Generic Makefile v2.6 ##

NAME = Koder
TYPE = APP
APP_MIME_SIG = x-vnd.KapiX-Koder

SRCS = \
	src/main.cpp \
	src/App.cpp \
	$(wildcard src/bookmarks/*.cpp) \
	$(wildcard src/controls/*.cpp) \
	$(wildcard src/editor/*.cpp) \
	$(wildcard src/find/*.cpp) \
	$(wildcard src/preferences/*.cpp) \
	$(wildcard src/support/*.cpp) \

RDEFS = Koder.rdef
LIBS = be tracker shared localestub scintilla yaml-cpp $(STDCPPLIBS)

LIBPATHS = $(shell findpaths -e -a $(shell uname -p) B_FIND_PATH_DEVELOP_LIB_DIRECTORY)
SYSTEM_INCLUDE_PATHS = \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/interface) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/shared) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/storage) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/tracker) \
	$(shell findpaths -e -a $(shell uname -p) B_FIND_PATH_HEADERS_DIRECTORY scintilla) \
	$(shell findpaths -e -a $(shell uname -p) B_FIND_PATH_HEADERS_DIRECTORY lexilla)

LOCAL_INCLUDE_PATHS = src/bookmarks src/controls src/editor src/find \
	src/preferences src/support
LOCALES = ca de el en en_AU en_GB es es_419 fr fur hu it ja nl pl pt ro ru sv tr uk zh_Hans

SYMBOLS := TRUE
DEBUGGER := TRUE
# -gno-column-info is a workaround for Debugger issue (#15159)
COMPILER_FLAGS = -gno-column-info -std=c++20 -Wall -Wextra -Werror

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

# TESTS

TEST_DIR := test

$(OBJ_DIR)/$(TEST_DIR)-%.o : $(TEST_DIR)/%.cpp
	$(C++) -c $< $(INCLUDES) $(CFLAGS) -o "$@"

TEST_SRCS = \
	main.cpp \
	TestUtils.cpp \
	TestFindReplace.cpp

TEST_OBJECTS = $(addprefix $(OBJ_DIR)/test-, $(addsuffix .o, $(foreach file, \
	$(TEST_SRCS), $(basename $(notdir $(file))))))

TEST_TARGET = $(TARGET_DIR)/$(NAME)_tests

TEST_BASE_OBJS = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

$(TEST_TARGET): $(TEST_BASE_OBJS) $(TEST_OBJECTS)
	$(LD) -o "$@" $(TEST_BASE_OBJS) $(TEST_OBJECTS) $(LDFLAGS) -lgtest

check : $(TEST_TARGET)
	$(TEST_TARGET)

check-debug : $(TEST_TARGET)
	Debugger $(TEST_TARGET)
