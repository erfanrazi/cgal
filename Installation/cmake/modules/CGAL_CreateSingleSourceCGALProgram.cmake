if(CGAL_CreateSingleSourceCGALProgram_included)
  return()
endif(CGAL_CreateSingleSourceCGALProgram_included)
set(CGAL_CreateSingleSourceCGALProgram_included TRUE)

include(CGAL_add_test)
include(CMakeParseArguments)

function(create_single_source_cgal_program firstfile )
  set(options NO_TESTING)
  set(oneValueArgs)
  set(multiValueArgs CXX_FEATURES)
  cmake_parse_arguments(create_single_source_cgal_program
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  set(CXX_FEATURES ${create_single_source_cgal_program_CXX_FEATURES})
  set(NO_TESTING ${create_single_source_cgal_program_NO_TESTING})

  if(NOT IS_ABSOLUTE "${firstfile}")
    set(firstfile "${CGAL_CURRENT_SOURCE_DIR}/${firstfile}")
  endif()

  get_filename_component(exe_name ${firstfile} NAME_WE)

  if(EXISTS "${firstfile}")

    if(CXX_FEATURES AND NOT COMMAND target_compile_features)
      message(STATUS "NOTICE: ${exe_name}.cpp requires a CMake version >= 3.1 to detect C++ features, and will not be compiled.")
      return()
    endif()
    if(CXX_FEATURES)
      set(MISSING_CXX_FEATURES ${CXX_FEATURES})
      if(CMAKE_CXX_COMPILE_FEATURES)
        list(REMOVE_ITEM MISSING_CXX_FEATURES ${CMAKE_CXX_COMPILE_FEATURES})
      endif()
    endif()
    # Now MISSING_CXX_FEATURES is the set CXX_FEATURES minus CMAKE_CXX_COMPILE_FEATURES
    if(MISSING_CXX_FEATURES)
      message(STATUS "NOTICE: ${exe_name}.cpp requires the C++ features \"${MISSING_CXX_FEATURES}\" and will not be compiled.")
      return()
    endif()

    set( all "${firstfile}" )

    # remaining files
    foreach( i ${create_single_source_cgal_program_UNPARSED_ARGUMENTS} )
      set( all ${all} ${CMAKE_CURRENT_SOURCE_DIR}/${i} )
    endforeach()

    add_executable(${exe_name} ${all})
    if(CXX_FEATURES)
      target_compile_features(${exe_name} PRIVATE ${CXX_FEATURES})
    endif()

    get_directory_property(folder_NO_TESTING CGAL_NO_TESTING)

    if(folder_NO_TESTING OR NOT BUILD_TESTING)
      set(NO_TESTING TRUE)
    endif()

    if(NOT NO_TESTING)
      cgal_add_test(${exe_name})
    endif(NOT NO_TESTING)

    add_to_cached_list( CGAL_EXECUTABLE_TARGETS ${exe_name} )

    # Link the executable to CGAL and third-party libraries
    if ( CGAL_AUTO_LINK_ENABLED )

      target_link_libraries(${exe_name} ${CGAL_3RD_PARTY_LIBRARIES} )

    else()

      target_link_libraries(${exe_name} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} )

    endif()
  else()
    message(AUTHOR_WARNING "The executable ${exe_name} will not be created because the source file ${firstfile} does not exist.")
  endif()

endfunction()
