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

#ifndef OPERATORS_HPP
#define OPERATORS_HPP
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/BasicBlock.h>
#include <vector>

class fcn_builder;
class arglist_builder;

struct op_info {
    llvm::Value* (*impl)(std::vector<llvm::Value*>&, const fcn_builder&, llvm::BasicBlock*);
    // NOTE: We used to have a visitor function that allows ops to interact
    // with arglist_builder by themselves so that they can request any argument
    // they'd like. But that was deemed as a overkill. For that to happen, we
    // had also to use a postorder iteration to visit all nodes.
    // Now all the requests are summed up below (well, only two). Both are
    // currently used only by DIFF_OP.
    // FIXME: These properties apply to each argument, rather than to the
    // entire operator. But it can be difficult to mitigate this with those
    // variadic operator, e.g. diff.
    const bool wants_callable = false;
    const bool iter_instead_of_val = false;
    static void init(llvm::LLVMContext& c, llvm::Module& m);
    llvm::Value* call(std::vector<llvm::Value*>& stack, const fcn_builder& am, llvm::BasicBlock* bb) {
        return impl(stack, am, bb);
    }
};

extern op_info PLUS_OP;
extern op_info MINUS_OP;
extern op_info MUL_OP;
extern op_info DIV_OP;
extern op_info POW_OP;
extern op_info DIFF_OP;

extern llvm::StringMap<op_info*> function_map;
#endif
