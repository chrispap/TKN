HEADERS += \
    lib/TKN_Util.h \
    lib/TKN_Queue.h \
    lib/TKN.h \
    lib/rs232.h \
    gui/TKN_Window.h \
    gui/TKN_Nodebox.h \
    lib/HEXParser.hpp \
    lib/Utility.hpp \
    lib/ErrorMsg.hpp

SOURCES += \
    gui/main.cpp \
    lib/TKN_Util.c \
    lib/TKN_Queue.c \
    lib/TKN.c \
    lib/rs232.c \
    lib/ListSerial.c \
    gui/TKN_Window.cpp \
    gui/TKN_Nodebox.cpp \
    lib/HEXParser.cpp \
    lib/Utility.cpp \
    lib/ErrorMsg.cpp

FORMS += \
    gui/TKN_Window.ui \
    gui/TKN_Nodebox.ui

#CONFIG += CONSOLE

include ( C:\Qwt-6.1.0\features\qwt.prf )

RESOURCES += \
    gui/resources.qrc
