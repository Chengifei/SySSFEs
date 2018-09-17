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
 *
 * This header encapsulates diagnostic issuing and propagating mechanism.
 */

#ifndef DIAGNOSTICS_HPP
#define DIAGNOSTICS_HPP
#include <ostream>
#include <vector>
#include <cstdio>

class DiagCtl {
public:
    std::ostream& os;
    bool show_color = true;
    bool show_src = true;
    bool show_carets = true;
    bool absolute_path = false;
    enum LEVEL {
        REMARK = 0,
        WARNING,
        ERROR
    };
    static const char* header[3];
    LEVEL mute;
    DiagCtl(std::ostream& os) : os(os) {}
    template <typename... Ts>
    void issue(LEVEL l, const char* fmt, Ts&&... msgs) {
        emit_type(l);
        std::vector<char> buf(std::snprintf(nullptr, 0, fmt, std::forward<Ts&&>(msgs)...));
        std::snprintf(buf.data(), buf.size(), fmt, std::forward<Ts&&>(msgs)...);
        os << buf.data() << "\n";
    }
private:
    void emit_reset() {
        if (show_color) os << "\x1b[0m";
    }
    void emit_src(const char* fn, std::size_t ln, int col_b, int col_e);
    void emit_type(LEVEL l) {
        if (show_color)
            os << "\x1b[31;1m" << header[l] << ": \x1b[0m";
        else
            os << header[l] << ": ";
    }
};
#endif
