cmake_minimum_required(VERSION 3.6)

project(tacSense CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g2 -Wall -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

find_package(Franka 0.8.0 REQUIRED )
find_package(Eigen3 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${EIGEN3_INCLUDE_DIR})

add_library(examples_common STATIC
    src/examples_common.cpp
)
target_link_libraries(examples_common PUBLIC Franka::Franka)

add_library(controller STATIC
    src/controller.cpp
)
target_link_libraries(controller PUBLIC pthread Franka::Franka)

set(APPS
    verticalSlideOnFabric
)

set(PCAN_SUPPORT OFF)

if(PCAN_SUPPORT)
    add_library(pcan_interface STATIC
    src/pcan_interface.cpp
    )
    target_link_libraries(pcan_interface PUBLIC stdc++fs pthread pcanbasic)
endif()

foreach(app ${APPS})
    add_executable(${app} src/${app}.cpp)
    target_link_libraries(${app} pthread Franka::Franka examples_common controller)
    if(PCAN_SUPPORT)
        target_link_libraries(${app} pcan_interface)
    endif()
endforeach()

include(GNUInstallDirs)
