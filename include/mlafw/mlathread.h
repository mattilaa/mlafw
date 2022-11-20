#ifndef __MLA_THREAD_H__
#define __MLA_THREAD_H__

#include <thread>
#include <assert.h>

namespace mla::thread {

/// Lightweight thread interface wrapping std::thread object
class Thread
{
    std::unique_ptr<std::thread> _thread;
public:
    virtual ~Thread() { assert(!_thread->joinable()); }

    // Starts the thread and runs execute function.
    virtual auto start() -> void
    { _thread = std::make_unique<std::thread>(&Thread::execute, this); }

    // Joins the thread.
    //
    // This should be called after exit is called
    // and as a last step before the object is destroyed.
    virtual auto join() -> void { if(_thread->joinable()) _thread->join(); }

    // Returns Id of the running thread
    //
    // \return Thread id
    auto getId() const -> std::thread::id { return _thread->get_id(); }

    // Returns native handle of the thread.
    //
    // \return Thread handle
    auto getNativeHandle() const -> std::thread::native_handle_type
    { return _thread->native_handle(); }

    /// Executor function of the worker thread.
    virtual auto execute() -> void = 0;

    // Request exit for thread.
    //
    // This function should break execution event loop, or clean up the object.
    virtual auto exit() -> void = 0;
};

} // namespace mla::thread

#endif
