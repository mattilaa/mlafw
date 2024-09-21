#ifndef __MLA_THREAD_H__
#define __MLA_THREAD_H__

#include <atomic>
#include <cassert>
#include <thread>

namespace mla::thread
{

class Thread
{
public:
    Thread() = default;
    virtual ~Thread()
    {
        assert(!(_thread && _thread->joinable()));
    }

    // Prevent copying and moving
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;

    virtual void start()
    {
        [[unlikely]] if(_thread) return;
        _thread = std::make_unique<std::thread>(&Thread::run, this);
    }

    virtual void join()
    {
        if(_thread && _thread->joinable())
        {
            _thread->join();
        }
    }

    [[nodiscard]] std::thread::id getId() const
    {
        return _thread ? _thread->get_id() : std::thread::id{};
    }

    [[nodiscard]] std::thread::native_handle_type getNativeHandle() const
    {
        return _thread ? _thread->native_handle()
                       : std::thread::native_handle_type{};
    }

    virtual void execute() = 0;
    virtual void exit() = 0;

protected:
    std::atomic<bool> _shouldExit{false};

private:
    std::unique_ptr<std::thread> _thread;

    void run() { execute(); }
};

}  // namespace mla::thread

#endif // __MLA_THREAD_H__
