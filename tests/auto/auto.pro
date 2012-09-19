TEMPLATE=subdirs
SUBDIRS=\
    linguist \
    host.pro \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \
    cmake

# These tests don't make sense for cross-compiled builds
cross_compile:SUBDIRS -= host.pro

# These tests need the QtHelp module
isEmpty(QT.help.name): SUBDIRS -= \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \
