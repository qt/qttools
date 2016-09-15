TEMPLATE=subdirs
SUBDIRS=\
    linguist \
    qdoc \
    qtattributionsscanner \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \
    cmake \
    installed_cmake \
    qtdiag \
    windeployqt \
    macdeployqt

installed_cmake.depends = cmake

# These tests don't make sense for cross-compiled builds
cross_compile:SUBDIRS -= linguist qdoc qtattributionsscanner windeployqt qhelpgenerator qtdiag

# Tests that might make sense, but currently use SRCDIR
cross_compile:SUBDIRS -= qhelpcontentmodel qhelpenginecore qhelpindexmodel qhelpprojectdata

# These tests need the QtHelp module
!qtHaveModule(help): SUBDIRS -= \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \

!qtConfig(process): SUBDIRS -= qtattributionsscanner linguist qtdiag windeployqt macdeployqt
!win32|winrt: SUBDIRS -= windeployqt
!macos: SUBDIRS -= macdeployqt
