/* Copyright 2017-2018 by Yifei Zheng
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TYPED_BUFFER_HPP
#define TYPED_BUFFER_HPP
#include <cstddef>
#include <type_traits>
#include <memory>

template <typename T, std::size_t sz>
class typed_buffer {
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type raw_type;
    std::unique_ptr<raw_type[]> buffer;
public:
    typed_buffer() {
        buffer = std::make_unique<raw_type[]>(sz);
    }
    T& operator[](std::size_t idx) {
        return *reinterpret_cast<T*>(&buffer[idx]);
    }
    T* begin() noexcept {
        return &operator[](0);
    }
    T* end() noexcept {
        return &operator[](sz);
    }
};

template <typename T>
class placeholder {
    alignas(alignof(T)) unsigned char buffer[sizeof(T)];
public:
    T* operator->() noexcept {
        return reinterpret_cast<T*>(buffer);
    }
    T& operator*() noexcept {
        return *reinterpret_cast<T*>(buffer);
    }
    T* get_addr() noexcept {
        return reinterpret_cast<T*>(buffer);
    }
};
#endif
