cmake_minimum_required(VERSION 3.5.0)

project(simpleMovieUi VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)

find_package(snc QUIET CONFIG)
if(NOT snc_FOUND)
    include(FetchContent)
    message(INFO "fetching snc git")
    FetchContent_Declare(
      snc
      GIT_REPOSITORY https://github.com/yorns/snc.git
      GIT_TAG        master
    )
    FetchContent_MakeAvailable(snc)
endif()

find_package(nlohmann_json 3.10.0 REQUIRED)
find_package(Boost 1.70.0 REQUIRED COMPONENTS regex system thread )
find_package(Curses)

find_path(snc_client_INCLUDE_DIRS NAMES snc/client.h)

find_library(NCURSESW_LIBRARIES NAMES ncursesw)
set(CURSES_NEED_NCURSES true)

set(UI_SRC 
        src/ui_main.cpp
        src/database.cpp
        src/database.h
        src/KeyHit.h
        src/Gui.cpp
        src/Controller.cpp
        src/Key.h
        src/Key.cpp
        src/Player.cpp
        src/OmxPlayer.cpp
        src/MPlayer.cpp
        src/Config.cpp
        src/Config.h
        src/getKey.cpp
        src/getKey.h
        )


# find_library(snc_client_LIBRARIES NAMES snc_client)

message(STATUS "using ncurses from ${NCURSESW_LIBRARIES}")

add_executable(ui ${UI_SRC})

if(ON_HOST)
    target_compile_definitions(ui PUBLIC RUN_ON_HOST)
endif()

target_include_directories(ui PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)  

target_compile_options(ui PUBLIC -Wall -Wextra -pedantic)

target_link_libraries(ui 
	PUBLIC
	Boost::boost
        Boost::regex
	snc::snc_client
        nlohmann_json::nlohmann_json
	${CURSES_LIBRARIES}
)

set_target_properties(ui PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_executable( keysender 
        src/keysender.cpp 
        src/Key.cpp 
        src/getKey.cpp 
        )

target_include_directories(keysender PUBLIC 
  ${CMAKE_SOURCE_DIR}/src
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

target_compile_options(keysender PUBLIC -Wall -Wextra -pedantic)
    target_link_libraries(keysender
        PUBLIC
        Boost::boost
        snc::snc_client
        ${CURSES_LIBRARIES}
)
set_target_properties(keysender PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            )

install(TARGETS ui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/snc
        )

install(FILES scripts/db_find_on_mount.sh
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE
        DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES scripts/start_ui.sh
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE
        DESTINATION ${CMAKE_INSTALL_BINDIR})
