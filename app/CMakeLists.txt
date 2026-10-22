cmake_minimum_required(VERSION 3.20)

set(APP_NAME Satyr-TTRPG)

file(GLOB INCLUDES
  "includes/AppCore/*.h"
  "includes/DisplaySystem/*.h"
  "includes/EventsSystem/*.h"
  "includes/Map/*.h"
  "includes/NetworkLevel/*.h"
  "includes/ViewInterface/*.h"
  "includes/GameState/*.h"
  "includes/UI/*.h"
  "includes/Tokens/*.h"
)

file(GLOB SOURCES
  "src/main.c"
  "src/AppCore/*.c"
  "src/DisplaySystem/*.c"
  "src/EventsSystem/*.c"
  "src/Map/*.c"
  "src/NetworkLevel/*.c"
  "src/ViewInterface/*.c"
  "src/GameState/*.c"
  "src/UI/*.c"
  "src/Tokens/*.c"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_executable(${APP_NAME} WIN32 ${SOURCES} ${INCLUDES})
else()
  add_executable(${APP_NAME} ${SOURCES} ${INCLUDES})
endif()

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_link_libraries(${APP_NAME} PRIVATE PkgConfig::GTK4 PkgConfig::EPOXY m)

add_subdirectory(${CMAKE_SOURCE_DIR}/externs/plibsys ${CMAKE_CURRENT_BINARY_DIR}/plibsys)
target_link_libraries(${APP_NAME} PRIVATE plibsys)

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/externs/stbi)
target_compile_options(${APP_NAME} PRIVATE ${GTK4_CFLAGS_OTHER})
set_target_properties(${APP_NAME} PROPERTIES ENABLE_EXPORTS ON)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/saves)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set_target_properties(${APP_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/lib"
  )
endif()


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building with ASan and LSan.")
    target_compile_options(${APP_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${APP_NAME} PRIVATE -fsanitize=address)
  endif()

  add_custom_command(
    TARGET ${APP_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../res
        $<TARGET_FILE_DIR:${APP_NAME}>/res
      COMMAND_EXPAND_LISTS
  )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(MSYS2_PREFIX "C:/msys64/ucrt64" CACHE PATH "MSYS2 UCRT64 prefix")

  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copy dependencies DLL ..."
    COMMAND ${MSYS2_PREFIX}/bin/ntldd.exe -R $<TARGET_FILE:${APP_NAME}>
              | grep -i "${MSYS2_PREFIX}"
              | awk -F '=>' '{print $2}'
              | awk '{print $1}'
              | xargs -I {} cp {} $<TARGET_FILE_DIR:${APP_NAME}>
              COMMENT "Copy ..."
  )
  
  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "GTK4 resources copy ..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
             ${MSYS2_PREFIX}/share
             $<TARGET_FILE_DIR:${APP_NAME}>/share
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${MSYS2_PREFIX}/etc
            $<TARGET_FILE_DIR:${APP_NAME}>/etc
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:${APP_NAME}>/res
  )

  install(DIRECTORY "${MSYS2_PREFIX}/share/" DESTINATION "share" COMPONENT Runtime)
  install(DIRECTORY "${MSYS2_PREFIX}/etc/" DESTINATION "etc" COMPONENT Runtime)
endif()


install(TARGETS ${APP_NAME} RUNTIME DESTINATION "." COMPONENT Runtime)

install(TARGETS plibsys
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "lib" 
        COMPONENT Runtime)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/res/" DESTINATION "res" COMPONENT Resources)

include(InstallRequiredSystemLibraries)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES $<TARGET_FILE:Satyr-TTRPG>
      RESOLVED_DEPENDENCIES_VAR _resolved_deps
      UNRESOLVED_DEPENDENCIES_VAR _unresolved_deps
      DIRECTORIES "C:/msys64/ucrt64/bin"
    )
    foreach(_file ${_resolved_deps})
      if(_file MATCHES "msys64")
        file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "${_file}")
      endif()
    endforeach()
  ]] COMPONENT Runtime)
elseif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES $<TARGET_FILE:Satyr-TTRPG>
      RESOLVED_DEPENDENCIES_VAR _resolved_deps
      UNRESOLVED_DEPENDENCIES_VAR _unresolved_deps
    )
    foreach(_file ${_resolved_deps})
      if(NOT _file MATCHES "^/lib/|/lib64/|ld-linux|libc\\.so|libpthread\\.so|libdl\\.so|libm\\.so|librt\\.so")
        file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "${_file}")
      endif()
    endforeach()
  ]] COMPONENT Runtime)
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "${APP_NAME}")
set(CPACK_PACKAGE_VERSION "0.0.1")
include(CPack)

