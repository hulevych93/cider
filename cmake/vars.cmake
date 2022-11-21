# vars.cmake

if(NOT DEFINED GUNIT_VARS_EXPORTED)
gunit_export_var(GUNIT_VARS_EXPORTED 1)

gunit_export_var(GUNIT_BINARY_DIR ${GUNIT_BINARY_DIR})
gunit_export_var(GUNIT_CMAKE_DIR ${GUNIT_CMAKE_DIR})
gunit_export_var(GUNIT_INCLUDE_DIR ${GUNIT_ROOT_DIR}/src)

#output
gunit_export_var(GUNIT_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
gunit_export_var(GUNIT_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
gunit_export_var(GUNIT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
