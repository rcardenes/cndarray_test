find_path(LIBEV_INCLUDE_DIR
    NAMES ev.h
    )

find_library(LIBEV_LIBRARY
    NAMES ev
    )

if(LIBEV_FOUND)
    set(LIBEV_LIBRARIES ${LIBEV_LIBRARY})
    set(LIBEV_INCLUDE_DIRS ${LIBEV_INCLUDE_DIR})
endif()

mark_as_advanced(LIBEV_INCLUDE_DIR LIBEV_LIBRARY)
