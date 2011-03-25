TEMPLATE=subdirs
SUBDIRS=\
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \

SUBDIRS += linguist
!cross_compile:SUBDIRS += host.pro
