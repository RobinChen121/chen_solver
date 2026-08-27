#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "chen_solver::chen_solver_core" for configuration "Release"
set_property(TARGET chen_solver::chen_solver_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(chen_solver::chen_solver_core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/chen_solver_core.lib"
  )

list(APPEND _cmake_import_check_targets chen_solver::chen_solver_core )
list(APPEND _cmake_import_check_files_for_chen_solver::chen_solver_core "${_IMPORT_PREFIX}/lib/chen_solver_core.lib" )

# Import target "chen_solver::chen_solver" for configuration "Release"
set_property(TARGET chen_solver::chen_solver APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(chen_solver::chen_solver PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/chen_solver.exe"
  )

list(APPEND _cmake_import_check_targets chen_solver::chen_solver )
list(APPEND _cmake_import_check_files_for_chen_solver::chen_solver "${_IMPORT_PREFIX}/bin/chen_solver.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
