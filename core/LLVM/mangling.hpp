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

#ifndef LLVM_MANGLING_HPP
#define LLVM_MANGLING_HPP
#include <string>

class mangler {
    unsigned rule_counter = 0;
public:
    std::string rule(std::string&& name) {
        return "_" + name + "_" + std::to_string(rule_counter++);
    }
    static std::string iter_inc(std::string ctrl) {
        return "_" + ctrl + "_inc";
    }
    static std::string iter_dec(std::string ctrl) {
        return "_" + ctrl + "_dec";
    }
};
#endif
