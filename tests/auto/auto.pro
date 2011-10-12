TEMPLATE=subdirs
SUBDIRS=\
    linguist \
    host.pro \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \

# These tests don't make sense for cross-compiled builds
cross_compile:SUBDIRS -= host.pro

# These tests don't make sense unless the tools will be built.
!contains(QT_BUILD_PARTS, tools): SUBDIRS -= \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \
