project (import_history)

# minimal required versions
cmake_minimum_required (VERSION 2.8)
set (QT_MIN_VERSION "4.7.0")
set (CMAKE_MIN_VERSION "2.8.0")

set (KADU_FIND_REQUIRED true)
include (FindKadu)

include (FindPkgConfig)

set (SOURCES
  import_history.cpp
  gui/gui.cpp
  importers/importer.cpp
  importers/gg7/crc32.cpp
  importers/gg7/ggimporter.cpp
  importers/gg8/gg8importer.cpp
  misc/memfile.cpp
)

set (MOC_SOURCES
  import_history.h
  gui/gui.h
  importers/importer.h
  importers/gg7/ggimporter.h
  importers/gg8/gg8importer.h
)

set (TRANSLATION_SOURCES
  translations/import_history_pl.ts
  translations/import_history_de.ts
  translations/import_history_it.ts
  translations/import_history_fr.ts
)

set (CONFIGURATION_FILES
)

if (WIN32)
  # we need to bundle sqlite on Windows
  list (APPEND SOURCES
    sqlite3.c
  )

  set_source_files_properties (sqlite3.c PROPERTIES
    COMPILE_FLAGS "-fno-strict-aliasing -Wno-error"
  )
endif (WIN32)

if (WIN32)
	if (NOT MINGW_LIBGADU_DIR)
		message (SEND_ERROR " Pass path to libgadu -DMINGW_LIBGADU_DIR=path")
		set (MINGW_LIBGADU_DIR "")
	else (NOT MINGW_LIBGADU_DIR)
		set (LIBGADU_FOUND true)
		set (LIBGADU_INCLUDE_DIRS ${MINGW_LIBGADU_DIR})
		set (LIBGADU_LIBRARY_DIRS ${MINGW_LIBGADU_DIR})
		set (LIBGADU_LIBRARIES gadu ws2_32)
	endif (NOT MINGW_LIBGADU_DIR)
else (WIN32)
	pkg_search_module (LIBGADU REQUIRED libgadu)
endif (WIN32)

include_directories (${LIBGADU_INCLUDE_DIRS})
link_directories (${LIBGADU_LIBRARY_DIRS})

qt4_wrap_ui(import_history_UIS gui/import_history.ui)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

kadu_PLUGIN(import_history
  PLUGIN_SOURCES ${SOURCES} ${import_history_UIS}
  PLUGIN_MOC_SOURCES ${MOC_SOURCES}
  PLUGIN_TRANSLATION_SOURCES ${TRANSLATION_SOURCES}
  PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
  PLUGIN_LIBRARIES ${LIBGADU_LIBRARIES}
  PLUGIN_DEPENDENCIES history gadu_protocol
)
