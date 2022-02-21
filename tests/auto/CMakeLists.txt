# Generated from auto.pro.

if(QT_FEATURE_process AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(linguist)
    add_subdirectory(qtattributionsscanner)
    add_subdirectory(qtdiag)
endif()
if(TARGET Qt::qdoc AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(qdoc)
endif()
if(TARGET Qt::Help AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(qhelpcontentmodel)
    add_subdirectory(qhelpenginecore)
    add_subdirectory(qhelpgenerator)
    add_subdirectory(qhelpindexmodel)
    add_subdirectory(qhelpprojectdata)
endif()
# special case begin
# add_subdirectory(cmake)
# if (TARGET Qt::Linguist)
# add_subdirectory(cmake/linguist)
# endif()
# special case end
