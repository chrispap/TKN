HEADERS += \
    lib/TKN_Util.h \
    lib/TKN_Queue.h \
    lib/TKN.h \
    lib/rs232.h \
    gui/TKN_Window.h \
    gui/TKN_Nodebox.h

SOURCES += \
    gui/main.cpp \
    lib/TKN_Util.c \
    lib/TKN_Queue.c \
    lib/TKN.c \
    lib/rs232.c \
    lib/ListSerial.c \
    gui/TKN_Window.cpp \
    gui/TKN_Nodebox.cpp

FORMS += \
    gui/TKN_Node.ui \
    gui/TKN_Window.ui \
    gui/TKN_Nodebox.ui

CONFIG += CONSOLE
