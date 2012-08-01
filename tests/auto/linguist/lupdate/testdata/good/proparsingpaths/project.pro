SOURCES += file*.cpp filter.cpp non-existing.cpp

include(sub/sub.pri)

TRANSLATIONS = project.ts
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
