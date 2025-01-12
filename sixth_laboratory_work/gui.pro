QT += core gui widgets

SOURCES += services/temperature_gui.cpp \
            services/temperature_graphs.cpp \
            services/database_manager.cpp

HEADERS += services/temperature_graphs.hpp \
            services/database_manager.hpp \
            lib/gnuplot-iostream.h
INCLUDEPATH += /opt/homebrew/opt/libpqxx/include \
                /opt/homebrew/opt/boost/include \
                /opt/homebrew/opt/qt@5/include \
                lib

LIBS += -L/opt/homebrew/opt/libpqxx/lib \
        -L/opt/homebrew/opt/libpq/lib -lpq \
        -L/opt/homebrew/opt/boost/lib \
        -lpqxx -lpq \
        -lboost_iostreams \
        -L/opt/homebrew/opt/qt@5/lib -framework QtWidgets -framework QtCore -framework QtGui
CONFIG += link_pkgconfig
PKGCONFIG += Qt5Widgets Qt5Core Qt5Gui

QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -std=gnu++17
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
