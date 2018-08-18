cmake_minimum_required(VERSION 3.1.0)

project(simpleMovieUi VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(UI_SRC src/ui_main.cpp
        src/database.cpp
        src/database.h
        src/KeyHit.h
        src/Gui.cpp
        src/Controller.cpp
        src/Key.h
        src/Player.cpp
        src/OmxPlayer.cpp
        src/MPlayer.cpp)

if(EXISTS /opt/boost/include/boost/function.hpp)
    set(BOOST_ROOT /opt/boost)
endif()

find_package(Boost 1.53.0 REQUIRED COMPONENTS system thread filesystem regex)

find_library(snc_client_LIBRARIES NAMES snc_client)
find_path(snc_client_INCLUDE_DIRS NAMES snc/client.h)
find_library(NCURSESW_LIBRARIES NAMES ncursesw)

message(STATUS "using ncurses from ${NCURSESW_LIBRARIES}")

add_executable(ui ${UI_SRC})
set_target_properties(ui PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )
if(ON_HOST)
    target_compile_definitions(ui PUBLIC RUN_ON_HOST)
endif()

#target_include_directories(ui PUBLIC /usr/local/include/lohmann)
target_include_directories(ui PUBLIC src /usr/local/include/lohmann ${snc_client_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_compile_options(ui PUBLIC -O0 -g)
target_link_libraries(ui ${NCURSESW_LIBRARIES} ${snc_client_LIBRARIES} ${Boost_LIBRARIES} pthread)

add_executable(nameDescriptionMatcher src/nameDescriptionMatcher.cpp src/database.cpp)
target_include_directories(nameDescriptionMatcher PUBLIC src)
set_target_properties(nameDescriptionMatcher PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )
target_link_libraries(nameDescriptionMatcher ${Boost_LIBRARIES} pthread)

add_executable(categorieAdd categorieAdd.cpp src/database.cpp)
target_include_directories(categorieAdd PUBLIC src)
set_target_properties(categorieAdd PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )
target_link_libraries(categorieAdd ${Boost_LIBRARIES} pthread)

add_executable(deinterlacePlayerTag src/deinterlacePlayerTag.cpp src/database.cpp)
target_include_directories(deinterlacePlayerTag PUBLIC src)
set_target_properties(deinterlacePlayerTag PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )
target_link_libraries(deinterlacePlayerTag ${Boost_LIBRARIES} pthread)


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