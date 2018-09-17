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

#include "driver.hpp"
#include <llvm/IR/Value.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <codegen/resolver.hpp>
#include "machine_gen.hpp"
#include "operators.hpp"
#include "operator_map.hpp"
#include "mangling.hpp"
#include "accessors.hpp"
#include "fcn_base.hpp"
#include "fcn_builder.hpp"
#include "exprVisitor.hpp"
#include "Program.hpp"
#include <algorithm>
#include <cstdlib>
#include <queue>
#include <memory>

using namespace codegen;
using support::var_designation;

inline double ptr2dbl(void* ptr) {
    union {
        void* p;
        double d;
    } a;
    a.p = ptr;
    return a.d;
}

Builder::Builder(Program& p, Module& m) : prog(p), mod(m) {
    // FIXME: Memory leak if called multiple times
    op_info::init(mod.Context, mod.llvm_module);
    machine_code_generator::initialize();
    mod.llvm_module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    globals.obj = mod.reg_type(prog.object_type);
    make_chunk_type();
}

llvm::Function* Builder::make_func(const fcn_base& fb) {
    return llvm::Function::Create(fb.make_functype(mod.Context), llvm::GlobalValue::ExternalLinkage, "", &mod.llvm_module);
}

void Builder::categorizing_pass(support::Expr& expr) {
    auto* const dbl_tp = llvm::Type::getDoubleTy(mod.Context);
    for (support::Expr& it : support::Expr_preorder_iter(expr))
        if (it.type == static_cast<long long>(NODE_TYPE::COMBFIELD)) {
            std::unique_ptr<cmb_info> info(reinterpret_cast<cmb_info*>(it.data));
            it.data = reinterpret_cast<char*>(new cmb_accessor{ info->idx, prog.object_type.get_mem_idx(info->mem_name) });
            it.type = ATOM_NODE_CMBFIELD;
        }
        else if (it.type == static_cast<long long>(NODE_TYPE::MEMFIELD)) {
            std::unique_ptr<mmb_info> info(reinterpret_cast<mmb_info*>(it.data));
            it.data = reinterpret_cast<char*>(prog[info->name] + prog.object_type.get_mem_idx(info->mem_name));
            it.type = ATOM_NODE_FIELD;
        }
        else if (it.type == static_cast<long long>(NODE_TYPE::VARIABLE)) {
            std::unique_ptr<std::string> info(reinterpret_cast<std::string*>(it.data));
            it.data = reinterpret_cast<char*>(prog.add_var(std::move(*info)));
            it.type = ATOM_NODE_TMPVAR;
        }
        else if (it.type == static_cast<long long>(NODE_TYPE::NUMBER)) {
            double d = ptr2dbl(it.data);
            it.data = reinterpret_cast<char*>(llvm::ConstantFP::get(dbl_tp, d));
            it.type = ATOM_NODE_CONST;
        }
}

fcn_base Builder::argument_pass(support::Expr& expr, const step& ctx) {
    if (ctx.size() > 1) {
        
    }
}

void Builder::populate_controls() {
    std::size_t idx = 0;
    llvm::Type* objp = globals.obj->getPointerTo();
    for (const auto& i : prog.varpool.controls) {
        if (i.tp.agg)
            throw; // control variables must be scalars
        switch (i.tp.base) {
            case support::type::INT:
            {
                llvm::Type* inte = llvm::Type::getInt32Ty(mod.Context);
                llvm::Constant* val = llvm::ConstantInt::get(inte, ptr2dbl(i.start));
                globals.controls[idx] = new llvm::GlobalVariable(mod.llvm_module, inte, false, llvm::GlobalValue::InternalLinkage, val);
                auto ft = llvm::FunctionType::get(objp, {objp}, false);
                globals.iter_dec[idx] = llvm::Function::Create(ft, llvm::GlobalValue::WeakAnyLinkage, "", &mod.llvm_module);
                globals.iter_inc[idx] = llvm::Function::Create(ft, llvm::GlobalValue::WeakAnyLinkage, "", &mod.llvm_module);
                break;
            }
            case support::type::REAL:
            {
                llvm::Type* dbl = llvm::Type::getDoubleTy(mod.Context);
                llvm::Constant* val = llvm::ConstantFP::get(dbl, ptr2dbl(i.start));
                globals.controls[idx] = new llvm::GlobalVariable(mod.llvm_module, dbl, false, llvm::GlobalValue::InternalLinkage, val);
                auto ft = llvm::FunctionType::get(objp, {objp}, false);
                globals.iter_dec[idx] = llvm::Function::Create(ft, llvm::GlobalValue::InternalLinkage, mangler::iter_dec(i.name), &mod.llvm_module);
                llvm::BasicBlock* bb = llvm::BasicBlock::Create(mod.Context, "", globals.iter_dec[idx]);
                bb->getInstList().push_back(llvm::ReturnInst::Create(mod.Context, val));
                globals.iter_inc[idx] = llvm::Function::Create(ft, llvm::GlobalValue::InternalLinkage, mangler::iter_inc(i.name), &mod.llvm_module);
                bb = llvm::BasicBlock::Create(mod.Context, "", globals.iter_inc[idx]);
                bb->getInstList().push_back(llvm::ReturnInst::Create(mod.Context, val));
                break;
            }
            default:
                throw; // FIXME: other types cannot be control variables
        }
        ++idx;
    }
}

const Builder::chunk_type& Builder::make_chunk_type() {
    // as this is called in ctor, use assert instead
    // guaranteed to be initialized
    if (!globals.chunk) {
        globals.chunk = llvm::StructType::create(mod.Context);
        llvm::Type* head = mod.get_ptr_type(globals.chunk);
        llvm::Type* array = llvm::ArrayType::get(globals.obj, chunk_len());
        globals.chunk->setBody(head, array, head);
    }
    return globals.chunk;
}

struct codegen_iter : iter_utils::non_trivial_end_iter<codegen_iter> {
    std::stack<support::Expr*> stack;
    support::Expr* current;
    codegen_iter(support::Expr& expr)
        : current(&expr) {
        while (current->type == ATOM_NODE_OP
               && !static_cast<op_info*>(current->op->op_data)->wants_callable) {
            stack.push(current);
            current = &current->op->args[0];
        }
    }
    void operator++() {
        if (stack.empty()) {
            current = nullptr;
            return;
        }
        ++current;
        if (current - stack.top()->op->args.data() < stack.top()->op->args.size()) {
            while (current->type == ATOM_NODE_OP
                   && !static_cast<op_info*>(current->op->op_data)->wants_callable) {
                stack.push(current);
                current = &current->op->args[0];
            }
        }
        else {
            current = stack.top();
            stack.pop();
        }
    }
    support::Expr& operator*() const {
        return *current;
    }
    bool exhausted() {
        return !current;
    }
};

std::pair<fcn_base, llvm::Function*> Builder::compile_expr(support::Expr& expr) {
    fcn_base args(globals);
    struct functor {
        bool& f;
        functor(bool& flag) : f(flag) {
            f = true;
        }
        ~functor() {
            f = false;
        }
    };
    bool flag = false;
    support::Expr_custom_const_iter<functor> iter(expr);
    for (auto& i : iter) {
        if (flag)
            switch (i.type) {
                case ATOM_NODE_FIELD:
                    args.obj_reqs.insert(
                        prog.get_base(reinterpret_cast<const support::id_type>(i.data)));
                    break;
                case ATOM_NODE_CMBFIELD:
                    args.cmb_reqs.insert(reinterpret_cast<const cmb_accessor*>(i.data)->idx);
                    break;
                case ATOM_NODE_TMPVAR:
                    args.tmp_reqs.insert(reinterpret_cast<const support::id_type>(i.data));
                    //FIXME: Issue warning, tmp var doesn't have history
                    break;
            }
        else
            switch (i.type) {
                case ATOM_NODE_FIELD:
                    args.obj_iters.insert(
                        reinterpret_cast<const support::id_type>(i.data));
                    break;
                case ATOM_NODE_CMBFIELD:
                    args.cmb_iters.insert(
                        reinterpret_cast<const cmb_accessor*>(i.data)->idx);
                    break;
                case ATOM_NODE_TMPVAR:
                    args.tmp_reqs.insert(reinterpret_cast<const support::id_type>(i.data)); //FIXME
                    break;
                case ATOM_NODE_OP:
                    if (static_cast<op_info*>(i.op->op_data)->iter_instead_of_val)
                        iter.emplace(flag);
                    break;
            }
    }
    llvm::Function* fcn = make_func(args);
    // Compile an expression into LLVM IR in the context of fcn_builder,
    // The context is basically what are left to be unknown (arguments passed
    // in individually), and what are to be calculated
    // we don't use a const iterator because we're deleting (freeing) arg fields
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(fcn->getContext(), "", fcn);
    std::vector<llvm::Value*> stack;
    fcn_builder& builder = static_cast<fcn_builder&>(args);
    for (support::Expr& it : codegen_iter(expr)) {
        if (it.type == ATOM_NODE_OP) {
            llvm::Value* ret = static_cast<op_info*>(it.op->op_data)->call(stack, builder, bb);
            stack.push_back(ret); // Push the return value into stack
        }
        else if (it.type == ATOM_NODE_CONST)
            stack.push_back(reinterpret_cast<llvm::Value*>(it.data));
        else if (it.type == ATOM_NODE_FIELD) {
            support::id_type ma(reinterpret_cast<support::id_type>(it.data));
            llvm::Argument* arg = fcn->arg_begin() + builder.get_iter(ma);
            unsigned indices[] = { static_cast<unsigned>(ma - prog.get_base(ma)) }; // LLVM wants unsigned
            auto val = llvm::ExtractValueInst::Create(arg, indices, "", bb);
            stack.push_back(val);
        }
        else if (it.type == ATOM_NODE_CMBFIELD) {
            std::unique_ptr<cmb_accessor> ma(reinterpret_cast<cmb_accessor*>(it.data));
            llvm::Argument* arg = fcn->arg_begin() + builder.get_cmb_iter(ma->idx);
            unsigned indices[] = { static_cast<unsigned>(ma->fn) }; // LLVM wants unsigned
            auto val = llvm::ExtractValueInst::Create(arg, indices, "", bb);
            stack.push_back(val);
        }
        else if (it.type == ATOM_NODE_TMPVAR) {
            llvm::Value* arg = fcn->arg_begin() + builder.lookup_tmp(reinterpret_cast<std::uintptr_t>(it.data));
            stack.push_back(arg);
        }
        else if (it.type == ATOM_NODE_COMPILED) {
            stack.push_back(reinterpret_cast<llvm::Value*>(it.data));
        }
    }
    bb->getInstList().push_back(llvm::ReturnInst::Create(fcn->getContext(), stack.back()));
    return {args, fcn};
}

struct der_iter : iter_utils::non_trivial_end_iter<der_iter> {
    typedef const support::Expr T;
    T* const base;
    T* current;
    std::stack<T*, std::vector<T*>> stack;
    std::array<std::uint8_t, 8> order {};
    der_iter(const support::Expr& expr) : base(&expr), current(&expr) {
        while (current->type < 0) {
            stack.push(current);
            current = current->op->args.data();
            if (current->op->op_data == &DIFF_OP)
                ++order[0];
        }
    }
    void operator++() {
        if (current->type < 0) {
            stack.push(current);
            current = current->op->args.data();
            if (current->op->op_data == &DIFF_OP)
                ++order[0];
        }
        else {
            ++current;
            while (!stack.empty() && current - stack.top()->op->args.data() == stack.top()->op->args.size()) {
                current = stack.top() + 1;
                stack.pop();
                if (!stack.empty() && static_cast<std::size_t>(current - stack.top()->op->args.data()) != stack.top()->op->args.size() && current->type < 0 && current->op->op_data == &DIFF_OP)
                    --order[0];
            }
        }
    }
    T& operator*() const {
        return *current;
    }
    bool exhausted() const {
        return current != base && stack.empty();
    }
};

static bool should_be_initialized(const support::order_t& order, const support::order_t& max) {
    return order[0] < max[0];
}

Rule Builder::derivative_pass(const support::Expr& expr, variable_pool& pool) {
    // In preparing the codegen::variable_pool, each individual field of
    // objects has to have a unique id. This is done by allocating `n` ids for
    // each object, where n is the number of fields each one has, after which
    // we append global variables and such. This however requires us to
    // maintain a new map between llvm-side id and codegen-side id.
    Rule vars;
    der_iter iter(expr);
    for (auto& i : iter) {
        switch (i.type) {
            case ATOM_NODE_FIELD:
                {
                auto field = reinterpret_cast<support::id_type>(i.data);
                vars.push_back({field, iter.order});
                prog.varpool.is_initialized(vars.back());
                pool.add(vars.back(), true, should_be_initialized(iter.order, {1}));
                break;
                }
            case ATOM_NODE_CMBFIELD:
                {
                auto& field = *reinterpret_cast<cmb_accessor*>(i.data);
                vars.push_back({field.idx, iter.order});
                pool.add(vars.back(), true, should_be_initialized(iter.order, {1}));
                break;
                }
            case ATOM_NODE_TMPVAR:
                {
                support::id_type var = *reinterpret_cast<std::size_t*>(i.data);
                vars.push_back({var, iter.order});
                pool.add(vars.back(), !prog.varpool.is_control(var), prog.varpool.is_initialized(vars.back()));
                }
        }
    }
    return vars;
}

int Builder::compile() {
    populate_controls();
    prog.canonicalize();
    // Solve for canonical update sequence
    variable_pool pool;
    std::vector<Rule> pack; 
    pack.reserve(prog.rules.size());
    for (support::Expr& e : prog.rules) {
        categorizing_pass(e);
        pack.push_back(derivative_pass(e, pool));
    }
    RuleResolver rr(pool, &*pack.begin(), &*pack.end());
    if (!rr.solve())
        return -1;
    mangler mgl;
    for (auto& expr : prog.rules) {
        // Each rule corresponds to a top-level function whose name is given by
        // the mangler `mgl`. Each rule, however, may have some dependent
        // routines that need to be passed to operators like `diff`. These
        // subroutines are compiled first, so that they're available when
        // required. This is done with a post-order partial iteration on all
        // op nodes that declare `wants_callable`, to avoid recursion.
        // Only top-level functions are stored in funcs. Subroutines are not
        // not unique'd and are generated on demand.
        mangler submgl;
        for (auto& node : support::Expr_postorder_iter(expr)) {
            if (node.type == ATOM_NODE_OP && static_cast<op_info*>(node.op->op_data)->wants_callable) {
                auto fcn_info(compile_expr(node.op->args[0]));
                fcn_info.second->setName(submgl.rule("subrule"));
                node.op->args[0].data = reinterpret_cast<char*>(fcn_info.second);
                node.op->args[0].type = ATOM_NODE_COMPILED;
                mod.add_function(std::move(fcn_info.first), fcn_info.second);
            }
        }
        auto fcn_info(compile_expr(expr));
        mod.add_function(std::move(fcn_info.first), fcn_info.second);
        fcn_info.second->setName(mgl.rule("rule"));
    }
    // Main function generation
    llvm::Type* inte = llvm::Type::getInt32Ty(mod.Context);
    llvm::FunctionType* main_ty = llvm::FunctionType::get(inte, {inte}, false);
    llvm::Function* main = llvm::Function::Create(main_ty, llvm::GlobalValue::ExternalLinkage, "_atom_main", &mod.llvm_module);
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(mod.Context, "", main);
    for (const auto& step : rr.get()) {
        for (std::size_t i = 0; i != step.size(); ++i) {
            
            if (std::any_of(step.var[i].order.begin(), step.var[i].order.end(),
                             [](int i) {return i;})) {
                // We solved for a derivative, broadcasting is required
                // FIXME llvm::CallInst::Create(globals.od_updater, {}, "", bb);
            }
        }
    }
    bb->getInstList().push_back(llvm::ReturnInst::Create(mod.Context, llvm::ConstantInt::get(inte, 0)));
    return mod.verify();
}
