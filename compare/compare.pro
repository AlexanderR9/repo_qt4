
TEMPLATE = app
TARGET = start
DEPENDPATH += .
INCLUDEPATH += . \
            $$PWD \
            $$PWD/../lib

include($$PWD/../lib/subdirs.pri)
include($$PWD/../lib/lib.pri)


# Input
HEADERS += mainform.h
FORMS += mainwidget.ui
SOURCES += main.cpp mainform.cpp
