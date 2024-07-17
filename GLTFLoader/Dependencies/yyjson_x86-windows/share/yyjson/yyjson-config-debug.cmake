#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "yyjson::yyjson" for configuration "Debug"
set_property(TARGET yyjson::yyjson APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(yyjson::yyjson PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/yyjson.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/yyjson.dll"
  )

list(APPEND _cmake_import_check_targets yyjson::yyjson )
list(APPEND _cmake_import_check_files_for_yyjson::yyjson "${_IMPORT_PREFIX}/debug/lib/yyjson.lib" "${_IMPORT_PREFIX}/debug/bin/yyjson.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
