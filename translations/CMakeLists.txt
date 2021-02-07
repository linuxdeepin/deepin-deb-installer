file(GLOB APP_TS_FILES LIST_DIRECTORIES false *.ts)

qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})

file(GLOB QM_FILES "${CMAKE_SOURCE_DIR}/*.qm")

install(FILES ${APP_QM_FILES} DESTINATION share/deepin-deb-installer/translations)

add_custom_target(QMFILES ALL DEPENDS ${APP_QM_FILES})

add_subdirectory(policy)
