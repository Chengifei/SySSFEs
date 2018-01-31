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
 * This header encapsulates LLVM backend.
 */

#ifndef MACHINE_GEN_HPP
#define MACHINE_GEN_HPP
#include <memory>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>

class machine_code_generator {
    /// This class is not strictly necessary. It serves as a firewall against
    /// potential LLVM interface changes. LLVM marked its pass manager is legacy
    /// yet it is still unclear when that will be depracted. Keep this class in
    /// synchoronize with llc.
    // This deals with X86 only.
    llvm::legacy::PassManager PM;
    std::unique_ptr<llvm::TargetMachine> target_machine;
    llvm::TargetOptions tar_opt;
public:
    machine_code_generator(const std::string& triple) {
        std::string err;
        const llvm::Target* t = llvm::TargetRegistry::lookupTarget(triple, err);
        if (!t)
            abort(); // FIXME: report error
        target_machine.reset(t->createTargetMachine(triple, "", "+avx", tar_opt, llvm::Reloc::Static));
    }
    void operator()(llvm::Module& M, llvm::raw_pwrite_stream& os) {
        auto& tar_m = *static_cast<llvm::LLVMTargetMachine*>(target_machine.get());
        tar_m.addPassesToEmitFile(PM, os, llvm::TargetMachine::CodeGenFileType::CGFT_ObjectFile);
        PM.run(M);
    }
    static void initialize() {
        LLVMInitializeX86Target();
        LLVMInitializeX86TargetInfo();
        LLVMInitializeX86TargetMC();
        LLVMInitializeX86AsmPrinter();
        LLVMInitializeX86AsmParser();
    }
};
#endif
