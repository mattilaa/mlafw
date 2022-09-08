#ifndef __MLA_APP_H__
#define __MLA_APP_H__

#include "mlathread.h"

namespace mla::app {

class MlaApp : public mla::thread::Thread
{
public:
    auto execute() -> void override
    {
    }
};

}
#endif
