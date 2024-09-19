# Create example lib for unit testing

set(SOURCE_LIST
    "${MlaFw_SOURCE_DIR}/src/mlatimer.cpp"
    )

set(HEADER_LIST
    "${MlaFw_SOURCE_DIR}/include/mlafw/mla.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mlaattributetuple.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mlacommon.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mladefs.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mlaevent.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mlatimer.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mlathread.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/mlaeventthread.h"
    "${MlaFw_SOURCE_DIR}/include/mlafw/detail/mlaeventdetail.h"
    )

add_library(mlafw
    ${SOURCE_LIST}
    ${HEADER_LIST}
    )

target_include_directories(mlafw PUBLIC
    "${MlaFw_SOURCE_DIR}/include"
    "${concurrentqueue_SOURCE_DIR}"
    )

