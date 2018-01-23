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
 * This header encapsulates clang diagnostic issuing and propagating
 * mechanism.
 */

#ifndef DIAGNOSTICS_HPP
#define DIAGNOSTICS_HPP
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>

class DiagCtl { // Simplified Clang Diagnostic Controller
    llvm::raw_ostream& os;
    std::unique_ptr<clang::DiagnosticsEngine> diag_engine;
public:
    DiagCtl(llvm::raw_ostream& os) : os(os) {
        clang::DiagnosticOptions* diag_opt = new clang::DiagnosticOptions;
        diag_opt->ShowColors = true;
        clang::TextDiagnosticPrinter* tdp = new clang::TextDiagnosticPrinter(os, diag_opt);
        clang::DiagnosticIDs* did(new clang::DiagnosticIDs);
        diag_engine = std::make_unique<clang::DiagnosticsEngine>(did, diag_opt, tdp);
    }
    template <std::size_t N, typename... Ts>
    void issue(const char(&fmt)[N], Ts&&... msgs) {
        unsigned note = diag_engine->getCustomDiagID(clang::DiagnosticsEngine::Error, fmt);
	clang::DiagnosticBuilder diag = diag_engine->Report(note);
        diag_concat_impl(diag, std::forward<Ts&&>(msgs)...);
    }
private:
    template <typename T, typename... Ts>
    void diag_concat_impl(clang::DiagnosticBuilder& diag, T&& arg1, Ts&&... args) {
        diag << arg1;
        diag_concat_impl(diag, std::forward<Ts&&>(args)...);
    }
    void diag_concat_impl(clang::DiagnosticBuilder& diag) {}
};
#endif
