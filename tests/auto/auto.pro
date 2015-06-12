TEMPLATE=subdirs
SUBDIRS=\
    linguist \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \
    cmake \
    installed_cmake \
    qtdiag \
    windeployqt

installed_cmake.depends = cmake

# These tests don't make sense for cross-compiled builds
cross_compile:SUBDIRS -= linguist

# These tests need the QtHelp module
!qtHaveModule(help): SUBDIRS -= \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \

android|ios|qnx|wince*|winrt*:SUBDIRS -= qtdiag
!win32|wince*|winrt*:SUBDIRS -= windeployqt
