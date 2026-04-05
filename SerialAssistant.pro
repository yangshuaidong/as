QT += widgets serialport
TARGET = SerialAssistant
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialconfigwidget.cpp \
    sendwidget.cpp \
    receivewidget.cpp \
    protocol/protocolparser.cpp \
    protocol/protocolconfigwidget.cpp \
    protocol/protocolmanager.cpp \
    utils/hexutil.cpp \
    utils/xlsxwriter.cpp \
    utils/xlsxreader.cpp \
    themes/thememanager.cpp \
    tools/bertestdialog.cpp

HEADERS += \
    mainwindow.h \
    serialconfigwidget.h \
    sendwidget.h \
    receivewidget.h \
    protocol/protocolparser.h \
    protocol/protocolconfigwidget.h \
    protocol/protocolmanager.h \
    protocol/protocolframe.h \
    utils/hexutil.h \
    utils/xlsxwriter.h \
    utils/xlsxreader.h \
    themes/thememanager.h \
    tools/bertestdialog.h

RESOURCES += resources.qrc

win32:RC_FILE = app.rc
