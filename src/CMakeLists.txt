
file(GLOB explicator_modules "Explicator_Module*cc")

add_library(explicator
    Explicator.cc
    ${explicator_modules}
    Files.cc
    String.cc
)
target_link_libraries(explicator
    PUBLIC Threads::Threads
    PRIVATE "${STD_FS_LIB}"
)

target_include_directories(explicator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS explicator
        EXPORT ExplicatorTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES Explicator.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

