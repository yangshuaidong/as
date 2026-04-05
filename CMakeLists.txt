cmake_minimum_required(VERSION 3.16)
project(SerialAssistant VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 自动查找 Qt5 或 Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort)

set(SOURCES
    main.cpp
    mainwindow.cpp
    serialconfigwidget.cpp
    sendwidget.cpp
    receivewidget.cpp
    protocol/protocolparser.cpp
    protocol/protocolconfigwidget.cpp
    protocol/protocolmanager.cpp
    utils/hexutil.cpp
    utils/xlsxwriter.cpp
    utils/xlsxreader.cpp
    themes/thememanager.cpp
    tools/bertestdialog.cpp
)

set(HEADERS
    mainwindow.h
    serialconfigwidget.h
    sendwidget.h
    receivewidget.h
    protocol/protocolparser.h
    protocol/protocolconfigwidget.h
    protocol/protocolmanager.h
    protocol/protocolframe.h
    utils/hexutil.h
    utils/xlsxwriter.h
    utils/xlsxreader.h
    themes/thememanager.h
    tools/bertestdialog.h
)

add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS}
    resources.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
)
