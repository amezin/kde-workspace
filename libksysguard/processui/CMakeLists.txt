
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
check_include_files(byteswap.h HAVE_BYTESWAP_H)

set(ksysguard_WIDGETS "ksysguard.widgets")

set(processui_LIB_SRCS
   ksysguardprocesslist.cpp
   ProcessFilter.cpp
   ProcessModel.cpp
   ReniceDlg.cpp
   KTextEditVT.cpp
   scripting.cpp
)

qt5_wrap_ui( processui_LIB_SRCS
   ReniceDlgUi.ui
   ProcessWidgetUI.ui
)

add_library(processui ${processui_LIB_SRCS})
add_library(KF5::ProcessUi ALIAS processui)

if(X11_XRes_FOUND)
    target_link_libraries(processui PRIVATE ${X11_XRes_LIB} ${X11_LIBRARIES})
    include_directories(${X11_XRes_INCLUDE_PATH})
endif()

target_link_libraries(processui
    PUBLIC
        KF5::ProcessCore
        Qt5::Widgets
    PRIVATE
        Qt5::Script
        Qt5::WebKitWidgets
        KF5::KDE4Support
)
target_include_directories(processui
    PUBLIC
        "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/ksysguard>"
)

if(X11_FOUND)
    target_link_libraries(processui PRIVATE Qt5::X11Extras KF5::WindowSystem)
endif()

set_target_properties(processui
   PROPERTIES VERSION ${LIBKSYSGUARD_VERSION_STRING}
   SOVERSION ${LIBKSYSGUARD_VERSION_MINOR}
   EXPORT_NAME ProcessUi
)

install(TARGETS processui EXPORT libksysguardLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

# kde4_add_widget_files(ksysguardwidgets_PART_SRCS ${ksysguard_WIDGETS})
#
# add_library(ksysguardwidgets MODULE ${ksysguardwidgets_PART_SRCS})
#
# target_link_libraries(ksysguardwidgets ${KDE4_KDEUI_LIBS} processui)
#
# install(TARGETS ksysguardwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

install( FILES ProcessModel.h ProcessFilter.h KTextEditVT.h ksysguardprocesslist.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard/processui COMPONENT Devel)

