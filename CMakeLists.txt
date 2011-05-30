project (import_history)

# minimal required versions
cmake_minimum_required (VERSION 2.8)
set (QT_MIN_VERSION "4.7.0")
set (CMAKE_MIN_VERSION "2.8.0")

set (KADU_FIND_REQUIRED true)
include (FindKadu)

include_directories (${KADU_INCLUDE_DIR}/kadu-core ${CMAKE_CURRENT_BINARY_DIR})

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

qt4_wrap_ui(import_history_UIS gui/import_history.ui)

kadu_PLUGIN(import_history
  PLUGIN_SOURCES ${SOURCES} ${import_history_UIS}
  PLUGIN_MOC_SOURCES ${MOC_SOURCES}
  PLUGIN_TRANSLATION_SOURCES ${TRANSLATION_SOURCES}
  PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
  PLUGIN_DEPENDENCIES history
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gadu_protocol/)

target_link_libraries (import_history)
