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
 *
 * This header should contain only compile time routines, e.g. constexpr
 * variables, functions, as well as plain old macros. All symbols defined in
 * this header shall starts with 'ATOM_'.
 */

// Detect AVX support
#ifdef __AVX2__
#define ATOM_AVX 2
#elif defined __AVX__
#define ATOM_AVX 1
#endif

// Check if we need to zeroupper
#ifdef ATOM_AVX
#define ATOM_NEED_ZEROUPPER 1
#endif

// Detect FMA support
#ifdef __FMA__
#define ATOM_FMA 1
#endif

// Use reciprocals as a optimization for divisions?
#define ATOM_RCP_AS_DIV 1

// Check if we're compiling with Visual C++ (CL)
#ifdef _MSC_VER
#define ATOM_MSVC 1
#endif

// How many threads are available or preferable? (This is currently unimplemented)
#define ATOM_MAX_THREADS 4
