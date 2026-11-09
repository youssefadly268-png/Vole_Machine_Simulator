cmake_minimum_required(VERSION 3.19)
project(GUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    library/Cell.cpp
    library/CPU.cpp
    library/Instructions.cpp
    library/Machine.cpp
    library/ALU.cpp
    library/UI.cpp
    library/Register.cpp
    library/Memory.cpp
    library/Loader.cpp
    library/Vole_Exception.cpp
    library/Cell.h
    library/CPU.h
    library/Instructions.h
    library/Machine.h
    library/ALU.h
    library/UI.h
    library/Register.h
    library/Memory.h
    library/Vole_Exception.h
)

qt_add_executable(GUI
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
)
target_include_directories(GUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/library
)
target_link_libraries(GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS GUI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET GUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
