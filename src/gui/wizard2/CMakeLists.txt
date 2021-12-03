add_library(wizard SHARED
    progress.cpp progress.ui
    serverpage.cpp serverpage.ui
    wizard.cpp wizard.ui
    oauthpage.cpp oauthpage.ui
    spinpage.cpp spinpage.ui
    wizardpage.cpp

    jobs/resolveurl.cpp

    # TODO: meh
    ${PROJECT_SOURCE_DIR}/src/3rdparty/QProgressIndicator/QProgressIndicator.cpp
)

target_link_libraries(wizard PRIVATE libsync Qt5::Gui)

GENERATE_EXPORT_HEADER(wizard
        EXPORT_MACRO_NAME WIZARD_EXPORT
        EXPORT_FILE_NAME wizard_exports.h
)

set_target_properties(
  wizard
    PROPERTIES
      OUTPUT_NAME
        "${APPLICATION_EXECUTABLE}_wizard"
      VERSION
        ${LIBRARY_VERSION}
      SOVERSION
        ${LIBRARY_SOVERSION}
      AUTOUIC ON
      AUTORCC ON
)

target_compile_definitions(wizard PRIVATE
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
)

target_include_directories(wizard PRIVATE ${PROJECT_SOURCE_DIR}/src/3rdparty/QProgressIndicator/)
