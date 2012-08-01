include(win/win.pri)
include(mac/mac.pri)
include(unix/unix.pri)
include (common/common.pri)             # Important: keep the space before the '('
include(relativity/relativity.pri)

message($$SOURCES)

TRANSLATIONS = project.ts
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
