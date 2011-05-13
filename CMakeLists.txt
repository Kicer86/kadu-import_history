project (import_history)

# minimal required versions
cmake_minimum_required (VERSION 2.8)
set (QT_MIN_VERSION "4.7.0")
set (CMAKE_MIN_VERSION "2.8.0")

set (KADU_FIND_REQUIRED true)
include (FindKadu)

include_directories (${KADU_INCLUDE_DIR}/kadu-core)

set (SOURCES
  import_history.cpp
  crc32.cpp
  gui/import.cpp
  importers/importer.cpp
  importers/ggimporter.cpp
  classes/memfile.cpp
)

set (MOC_SOURCES
  import_history.h
  gui/import.h
  importers/importer.h
  importers/ggimporter.h
  classes/memfile.h
)

set (TRANSLATION_SOURCES
  translations/import_history_pl.ts
  translations/import_history_de.ts
  translations/import_history_it.ts
  translations/import_history_fr.ts
)

set (CONFIGURATION_FILES
)

kadu_PLUGIN(import_history
  PLUGIN_SOURCES ${SOURCES}
  PLUGIN_MOC_SOURCES ${MOC_SOURCES}
  PLUGIN_TRANSLATION_SOURCES ${TRANSLATION_SOURCES}
  PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
  PLUGIN_DEPENDENCIES history
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gadu_protocol/)

target_link_libraries (import_history)
