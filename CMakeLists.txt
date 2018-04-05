cmake_minimum_required(VERSION 3.1.0)

include(GNUInstallDirs)

set(UI_SRC startNcurses5.cpp database.cpp database.h KeyHit.h Gui.cpp Controller.cpp Key.h)

if(EXISTS /opt/boost/include/boost/function.hpp)
    set(BOOST_ROOT /opt/boost)
endif()

find_package(Boost 1.53.0 REQUIRED COMPONENTS system thread)

find_library(snc_client_LIBRARIES NAMES snc_client)
find_path(snc_client_INCLUDE_DIRS NAMES snc/client.h)
find_library(NCURSESW_LIBRARIES NAMES ncurses)

message(STATUS "using ncurses from ${NCURSESW_LIBRARIES}")

add_executable(ui ${UI_SRC})
set_target_properties(ui PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

#target_include_directories(ui PUBLIC /usr/local/include/lohmann)
target_include_directories(ui PUBLIC /usr/local/include/lohmann ${snc_client_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_compile_options(ui PUBLIC -O0 -g)
target_link_libraries(ui ${NCURSESW_LIBRARIES} ${snc_client_LIBRARIES} ${Boost_LIBRARIES} pthread)

install(TARGETS ui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/snc
        )