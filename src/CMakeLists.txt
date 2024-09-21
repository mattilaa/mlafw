# Create example lib for unit testing

set(SOURCE_LIST
    "${MlaFw_SOURCE_DIR}/src/mlatimer.cpp"
    )

set(HEADER_LIST
    "${MlaFw_SOURCE_DIR}/include/mlafw/mla.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/attributetuple.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/common.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/timer.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/thread.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/eventthread.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/detail/tupleutil.h"
    )

add_library(mlafw
    ${SOURCE_LIST}
    ${HEADER_LIST}
    )

target_include_directories(mlafw PUBLIC
    "${MlaFw_SOURCE_DIR}/include"
    "${concurrentqueue_SOURCE_DIR}"
    )

