# Create header-only library target

set(HEADER_LIST
    "${MlaFw_SOURCE_DIR}/include/mlafw/mla.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/attributetuple.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/common.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/eventthread.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/timer.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/thread.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/arrayquickmap.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/vectorquickmap.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/detail/tupleutil.h"
    )

add_library(mlafw INTERFACE)

target_include_directories(mlafw INTERFACE
    $<BUILD_INTERFACE:${MlaFw_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

target_link_libraries(mlafw INTERFACE concurrentqueue)

# Optional: Install headers
install(DIRECTORY ${MlaFw_SOURCE_DIR}/include/mlafw
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

# Optional: Install the target
install(TARGETS mlafw
        EXPORT MlaFwTargets
        INCLUDES DESTINATION include)
