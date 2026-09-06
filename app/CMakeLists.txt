cmake_minimum_required(VERSION 3.10)

set(APP_NAME Satyr-TTRPG)

file(GLOB INCLUDES
  "includes/AppCore/*.h"
  "includes/EventsSystem/*.h"
  "includes/Map/*.h"
  "includes/NetworkLevel/*.h"
  "includes/ViewInterface/*.h"
  "includes/GameState/*.h"
  "includes/UI/*.h"
)

file(GLOB SOURCES
  "src/main.c"
  "src/AppCore/*.c"
  "src/EventsSystem/*.c"
  "src/Map/*.c"
  "src/NetworkLevel/*.c"
  "src/ViewInterface/*.c"
  "src/GameState/*.c"
  "src/UI/*.c"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

  find_package(PkgConfig REQUIRED)

  pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
  pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

  add_executable(${APP_NAME} ${SOURCES} ${INCLUDES})
  target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

  #Profiling only for Linux
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")

    message(STATUS "Building with ASan and LSan.")

    target_compile_options(${APP_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${APP_NAME} PRIVATE -fsanitize=address)

  endif()

  target_link_libraries(${APP_NAME} PRIVATE PkgConfig::GTK4 PkgConfig::EPOXY m)

  add_subdirectory(${CMAKE_SOURCE_DIR}/externs/plibsys ${CMAKE_CURRENT_BINARY_DIR}/plibsys)
  target_link_libraries(${APP_NAME} PRIVATE plibsys)

  target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/externs/stbi)

  target_compile_options(${APP_NAME} PRIVATE ${GTK4_CFLAGS_OTHER})

  add_custom_command(
    TARGET ${APP_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../res
        $<TARGET_FILE_DIR:${APP_NAME}>/res
      COMMAND_EXPAND_LISTS
  )
  set_target_properties(${APP_NAME} PROPERTIES ENABLE_EXPORTS ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

  find_package(PkgConfig REQUIRED)

  pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
  pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

  add_executable(${APP_NAME} WIN32 ${SOURCES} ${INCLUDES})
  target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

  target_link_libraries(${APP_NAME} PRIVATE PkgConfig::GTK4 PkgConfig::EPOXY m)
  
  add_subdirectory(${CMAKE_SOURCE_DIR}/externs/plibsys ${CMAKE_CURRENT_BINARY_DIR}/plibsys)
  target_link_libraries(${APP_NAME} PRIVATE plibsys)

  target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/externs/stbi)

  target_compile_options(${APP_NAME} PRIVATE ${GTK4_CFLAGS_OTHER})
  add_custom_command(
    TARGET ${APP_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../res
        $<TARGET_FILE_DIR:${APP_NAME}>/res
      COMMAND_EXPAND_LISTS
  )
  set_target_properties(${APP_NAME} PROPERTIES ENABLE_EXPORTS ON)
endif()
