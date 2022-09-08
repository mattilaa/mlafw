#ifndef __MLA_DEFS_H__
#define __MLA_DEFS_H__

#if defined(__GNUC__)
    #define __MLA_LIKELY(x)     __builtin_expect((x), true)
    #define __MLA_UNLIKELY(x)   __builtin_expect((x), false)
#else
#error Platform not supported!
#endif

#endif
