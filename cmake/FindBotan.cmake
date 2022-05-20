find_path(Botan_INCLUDE_DIR botan/botan.h
        HINTS ${PC_BOTAN_INCLUDEDIR} ${PC_BOTAN_INCLUDE_DIRS} "c:/"
        PATH_SUFFIXES botan/include/botan-2
        )

find_library(Botan_LIBRARY NAMES botan
        HINTS ${PC_BOTAN_LIBDIR} ${PC_BOTAN_LIBRARY_DIRS} "c:/"
        PATH_SUFFIXES botan/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Botan DEFAULT_MSG
        Botan_LIBRARY Botan_INCLUDE_DIR)
mark_as_advanced(Botan_FOUND Botan_INCLUDE_DIR Botan_LIBRARY)

if (Botan_FOUND)
    get_filename_component(Botan_INCLUDE_DIRS ${Botan_INCLUDE_DIR} DIRECTORY)
endif ()

if (Botan_FOUND AND NOT TARGET Botan::Botan)
    add_library(Botan::Botan STATIC IMPORTED)
    if (MSVC)
        target_compile_options(Botan::Botan INTERFACE /wd4250)
    endif (MSVC)
    target_include_directories(Botan::Botan
            INTERFACE
            ${Botan_INCLUDE_DIR})
    set_target_properties(Botan::Botan PROPERTIES
            IMPORTED_LOCATION ${Botan_LIBRARY}
            )
endif ()