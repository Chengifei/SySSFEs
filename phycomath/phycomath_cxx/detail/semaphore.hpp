#include <condition_variable>
#include <mutex>
#include <atomic>
#include <bitset>

template <unsigned N>
class controller {
    std::mutex mtx;
    class Semaphore {
        std::condition_variable condition;
        unsigned count;
    public:
        Semaphore(unsigned count) : count(count) {}
        void notify() {
            ++count;
            condition.notify_one();
        }
        void wait(std::unique_lock<std::mutex>& lock) {
            while (!count)
                condition.wait(lock);
            --count;
        }
    };
    struct resource {
    private:
        controller& c;
        unsigned idx;
    public:
        resource(controller& c, unsigned idx) : c(c), idx(idx) {}
        resource(const resource&) = delete;
        resource& operator=(const resource&) = delete;
        resource(resource&& rhs) : c(rhs.c), idx(rhs.idx) {
            rhs.idx = -1U;
        }
        resource& operator=(resource&& rhs) {
            c = rhs.c;
            idx = rhs.idx;
            rhs.idx = -1U;
        }
        unsigned index() const noexcept {
            return idx;
        }
        ~resource() {
            if (idx != -1U)
                c.return_resource(idx);
        }
    };
    struct full_resource {
    private:
        controller* c;
    public:
        full_resource(controller* c) : c(c) {}
        full_resource(const full_resource&) = delete;
        full_resource& operator=(const full_resource&) = delete;
        full_resource(full_resource&& rhs) : c(rhs.c) {
            rhs.c = nullptr;
        }
        full_resource& operator=(full_resource&& rhs) {
            c = rhs.c;
            rhs.c = nullptr;
        }
        ~full_resource() {
            if (c)
                c->unlock_all();
        }
    };
    Semaphore sema;
    std::bitset<N> stat;
public:
    controller() : sema(N), stat(0) {}
    resource get_resource() {
        std::unique_lock<decltype(mtx)> lock(mtx);
        sema.wait(lock);
        for (unsigned i = 0; i != N; ++i)
            if (!stat[i]) {
                stat[i] = 1;
                return resource(*this, i);
            }
        throw;
    }
    full_resource lock_all() {
        mtx.lock();
        return full_resource(this);
    }
private:
    void return_resource(unsigned i) {
        std::unique_lock<decltype(mtx)> lock(mtx);
        stat[i] = 0;
        sema.notify();
    }
    void unlock_all() {
        mtx.unlock();
    }
};
