#ifndef __MLA_COMMON_H__
#define __MLA_COMMON_H__

#include <atomic>

namespace mla {

struct Id
{
    [[nodiscard]] static auto getId() -> unsigned
    {
        static std::atomic<unsigned> id {0};
        return id.fetch_add(1, std::memory_order_relaxed);
    }
};

}

#endif
