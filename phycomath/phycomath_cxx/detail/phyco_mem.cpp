#include "phyco_mem.hpp"

namespace utils {

std::vector<void*> memory_manager::pool;
quad_list memory_manager::data;

void* memory_manager::search_size(std::size_t size, unsigned align) {
    if (auto a = data.pop(size, align)) {
        return reinterpret_cast<void*>(a);
    }
    else {
        get_new(std::max<std::size_t>(size, PHYCO_DEFAULT_POOL_SIZE));
        return memory_manager::search_size(size, align);
    }
}

void memory_manager::get_new(std::size_t size) {
    pool.push_back((void*) new char[size]);
    data.push(intervalp((uintptr_t)pool.back(), (uintptr_t)pool.back() + size));
}

}