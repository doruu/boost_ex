cmake_minimum_required(VERSION 3.1)

project(boost_ex LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DBOOST_LOG_DYN_LINK)

find_package(Boost REQUIRED)
find_package(Qt5 COMPONENTS Core Quick REQUIRED)



add_executable(${PROJECT_NAME} "ftpclient.cpp" "ftpclient.h" "asioex.cpp" "asioex.h"  "uicontrol.cpp" "uicontrol.h" "logex.cpp" "logex.h"  "main.cpp" "qml.qrc")
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG> )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Quick boost_log-mt boost_system )
