#! [qt_add_translation]
qt_add_translation(qmFiles helloworld_en.ts helloworld_de.ts)
#! [qt_add_translation]

#! [qt_add_translation_output_location]
set(TS_FILES helloworld_en.ts helloworld_de.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "l10n")
qt_add_translation(qmFiles ${TS_FILES})
#! [qt_add_translation_output_location]

#! [qt_create_translation]
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} helloworld_en.ts helloworld_de.ts)
#! [qt_create_translation]
