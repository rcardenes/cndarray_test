#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <cassert>
#include <mutex>
#include <condition_variable>

namespace std {
    class binary_semaphore {
    public:
        explicit binary_semaphore( std::ptrdiff_t desired )
            : value{desired}
        {
            assert(desired == 0 || desired == 1);
        }
        binary_semaphore(const binary_semaphore&) = delete;

        void release() {
            std::unique_lock<std::mutex> lock(mx);

            if (value == 0) {
                value = 1;
                cv.notify_all();
            }

            lock.unlock();
        }

        void acquire() {
            std::unique_lock<std::mutex> lock{mx};

            if (value == 0) {
                cv.wait(lock, [&]() { return value > 0; });
            }

            value = 0;
            lock.unlock();
        }

        bool try_acquire() {
            std::lock_guard<std::mutex> guard{mx};

            if (value == 0) {
                return false;
            } else  {
                value = 0;
                return true;
            }
        }
    private:
        std::ptrdiff_t value;
        std::condition_variable cv;
        std::mutex mx;
    };
}

#endif // __SEMAPHORE_H__
