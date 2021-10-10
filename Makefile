## Haiku Generic Makefile v2.6 ##

NAME = Koder
TYPE = APP
APP_MIME_SIG = x-vnd.KapiX-Koder

SRCS = \
	src/main.cpp \
	src/App.cpp \
	$(wildcard src/controls/*.cpp) \
	$(wildcard src/support/*.cpp) \
	$(wildcard src/windows/*.cpp)

RDEFS = Koder.rdef
LIBS = be tracker shared localestub scintilla yaml-cpp $(STDCPPLIBS)

LIBPATHS = $(shell findpaths -e -a $(shell uname -p) B_FIND_PATH_DEVELOP_LIB_DIRECTORY)
SYSTEM_INCLUDE_PATHS = \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/interface) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/shared) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/storage) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/tracker) \
	$(shell findpaths -e -a $(shell uname -p) B_FIND_PATH_HEADERS_DIRECTORY scintilla)

LOCAL_INCLUDE_PATHS = src/controls src/support src/windows
LOCALES = ca de en en_GB es fr fur hu it ja nl pl pt ro ru sv tr uk

SYMBOLS := FALSE
DEBUGGER := FALSE
COMPILER_FLAGS = -std=c++17

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
