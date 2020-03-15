exists($$PWD/qhttp/qhttp.pro) {
    exists($$PWD/http-parser/http_parser.c) {
        DEFINES += LIB_QHTTP
    }
}

contains(DEFINES, LIB_QHTTP) {
    DEFINES += QHTTP_EXPORT

    INCLUDEPATH += \
        $$PWD/qhttp/src \
        $$PWD/qhttp/src/private

    HEADERS += \
        $$files($$PWD/qhttp/src/private/*.hpp) \
        $$files($$PWD/qhttp/src/private/*.hxx)

    SOURCES += \
        $$PWD/qhttp/src/qhttpabstracts.cpp \
        $$PWD/qhttp/src/qhttpserverconnection.cpp \
        $$PWD/qhttp/src/qhttpserverrequest.cpp \
        $$PWD/qhttp/src/qhttpserverresponse.cpp \
        $$PWD/qhttp/src/qhttpserver.cpp

    HEADERS += \
        $$PWD/qhttp/src/qhttpfwd.hpp \
        $$PWD/qhttp/src/qhttpabstracts.hpp \
        $$PWD/qhttp/src/qhttpserverconnection.hpp \
        $$PWD/qhttp/src/qhttpserverrequest.hpp \
        $$PWD/qhttp/src/qhttpserverresponse.hpp \
        $$PWD/qhttp/src/qhttpserver.hpp

    ###

    INCLUDEPATH += \
        $$PWD \
        $$PWD/http-parser

    SOURCES += \
        $$PWD/http-parser/http_parser.c

    HEADERS += \
        $$PWD/http-parser/http_parser.h
}
