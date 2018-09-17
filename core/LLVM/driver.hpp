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

#ifndef LLVM_DRIVER_HPP
#define LLVM_DRIVER_HPP
#include <type_traits>
#include "module.hpp"

struct Program;
struct fcn_base;

namespace codegen {
    class Rule;
    class variable_pool;
    class step;
}

namespace support {
    struct Expr;
    struct var_designation;
}

class Builder {
    Program& prog;
    Module& mod;
    env_t globals {};
    typedef env_t::combination_type combination_type;
    typedef env_t::chunk_type chunk_type;
public:
#pragma GCC visibility push(default)
    Builder(Program&, Module&);
    // FIXME: This probably shouldn't be compile-time constant
    typedef std::integral_constant<std::size_t, 1 << 20> chunk_size;
    int compile();
#pragma GCC visibility pop
public:
    std::size_t chunk_len() const {
        return (chunk_size::value - mod.dl.getPointerSize() * 2) / mod.dl.getTypeAllocSize(globals.obj);
    }
private:
    void populate_controls();
    const chunk_type& make_chunk_type();
    llvm::PointerType* hist_type();
    const combination_type& make_comb_type();
    void member_and_constant_pass(support::Expr&);
    codegen::Rule derivative_pass(const support::Expr&, codegen::variable_pool& /* out */);
    void categorizing_pass(support::Expr&);
    fcn_base argument_pass(support::Expr&, const codegen::step&);
    fcn_base argument_pass(support::Expr&, const support::var_designation&, const support::var_designation&);
    llvm::FunctionType* process_args(const fcn_base&);
    llvm::Function* make_func(const fcn_base&);
    std::pair<fcn_base, llvm::Function*> compile_expr(support::Expr&);
};
#endif
