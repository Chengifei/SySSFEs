/* Copyright 2018 by Yifei Zheng
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
#ifndef ARRAY_BUILDER_HPP
#define ARRAY_BUILDER_HPP
#include <vector>

template <typename T>
class custom_allocator {
public:
    void* const ptr;
    typedef T value_type;
    T* allocate(std::size_t) {
        return static_cast<T*>(ptr);
    }
    void deallocate(T*, std::size_t) {}
    void destroy(T*) {}
    template <typename U>
    operator custom_allocator<U>() {
        return { ptr };
    }
};

template <typename T>
class array_builder : public std::vector<T, custom_allocator<T>> {
public:
    array_builder(void* buffer) : std::vector<T, custom_allocator<T>>(custom_allocator<T>{buffer}) {}
};
#endif
