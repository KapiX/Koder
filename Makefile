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
	$(shell findpaths -e -a $(shell uname -p) B_FIND_PATH_HEADERS_DIRECTORY scintilla)

LOCAL_INCLUDE_PATHS = src/bookmarks src/controls src/editor src/find \
	src/preferences src/support
LOCALES = de en en_GB es fr pt pl ro ru uk

SYMBOLS := TRUE
DEBUGGER := TRUE
# -gno-column-info is a workaround for Debugger issue (#15159)
COMPILER_FLAGS = -gno-column-info -std=c++17

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
