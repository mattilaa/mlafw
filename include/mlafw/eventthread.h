#ifndef __MLA_EVENTTHREAD__
#define __MLA_EVENTTHREAD__

#include <iostream>
#include <thread>
#include <atomic>
#include <variant>
#include <optional>
#include <chrono>
#include <array>
#include <memory>

template<typename T, size_t Size>
class LockFreeQueue {
private:
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    static constexpr size_t MASK = Size - 1;

    std::array<std::atomic<T*>, Size> buffer;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};

public:
    LockFreeQueue() {
        for (auto& slot : buffer) {
            slot.store(nullptr, std::memory_order_relaxed);
        }
    }

    ~LockFreeQueue() {
        size_t head_pos = head.load(std::memory_order_relaxed);
        size_t tail_pos = tail.load(std::memory_order_relaxed);

        while (head_pos != tail_pos) {
            T* ptr = buffer[head_pos & MASK].load(std::memory_order_relaxed);
            delete ptr;
            head_pos++;
        }
    }

    bool push(const T& value) {
        T* new_value = new T(value);
        size_t current_tail = tail.load(std::memory_order_relaxed);

        while (true) {
            size_t next_tail = (current_tail + 1) & MASK;
            if (next_tail == head.load(std::memory_order_acquire)) {
                delete new_value;
                return false; // Queue is full
            }

            if (buffer[current_tail & MASK].load(std::memory_order_relaxed) != nullptr) {
                current_tail = tail.load(std::memory_order_relaxed);
                continue;
            }

            if (buffer[current_tail & MASK].compare_exchange_weak(
                    static_cast<T*>(nullptr), new_value,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                tail.store(next_tail, std::memory_order_release);
                return true;
            }

            current_tail = tail.load(std::memory_order_relaxed);
        }
    }

    std::optional<T> pop() {
        size_t current_head = head.load(std::memory_order_relaxed);

        while (true) {
            if (current_head == tail.load(std::memory_order_acquire)) {
                return std::nullopt; // Queue is empty
            }

            T* value = buffer[current_head & MASK].load(std::memory_order_relaxed);
            if (value == nullptr) {
                current_head = head.load(std::memory_order_relaxed);
                continue;
            }

            size_t next_head = (current_head + 1) & MASK;
            if (head.compare_exchange_weak(
                    current_head, next_head,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                T result = *value;
                delete value;
                buffer[current_head & MASK].store(nullptr, std::memory_order_relaxed);
                return result;
            }
        }
    }
};

// Event types
struct QuitEvent {};
struct PrintEvent { std::string message; };
struct DelayEvent { std::chrono::milliseconds duration; };

// Event variant
using Event = std::variant<QuitEvent, PrintEvent, DelayEvent>;

// Event loop class
class EventLoop {
private:
    LockFreeQueue<Event, 1024> queue;
    std::atomic<bool> running{true};
    std::thread worker;

    void processEvent(const Event& event) {
        std::visit([this](const auto& e) { this->handleEvent(e); }, event);
    }

    void handleEvent(const QuitEvent&) {
        running.store(false);
    }

    void handleEvent(const PrintEvent& e) {
        std::cout << "PrintEvent: " << e.message << std::endl;
    }

    void handleEvent(const DelayEvent& e) {
        std::this_thread::sleep_for(e.duration);
        std::cout << "DelayEvent: Slept for " << e.duration.count() << "ms" << std::endl;
    }

    void workerFunction() {
        while (running.load()) {
            auto event = queue.pop();
            if (event) {
                processEvent(*event);
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    EventLoop() : worker(&EventLoop::workerFunction, this) {}

    ~EventLoop() {
        if (running.load()) {
            stop();
        }
        worker.join();
    }

    bool pushEvent(const Event& event) {
        return queue.push(event);
    }

    void stop() {
        pushEvent(QuitEvent{});
    }
};


#endif
