/* Copyright 2017 by Yifei Zheng
 * This file is part of ATOM
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * This header governs all environment or platform dependent macros.
 * All platform-dependent header shall include this file directly or indirectly.
 */

// Detect AVX support
#ifdef __AVX2__
#define ATOM_AVX 2
#elif defined __AVX__
#define ATOM_AVX 1
#endif

// Check if we need to zeroupper
#ifdef ATOM_AVX
#define ATOM_NEED_ZEROUPPER
#endif

// Detect FMA support
#ifdef __FMA__
#define ATOM_FMA
#endif

// Use reciprocals as a optimization for divisions?
#define ATOM_RCP_AS_DIV

// Check if we're compiling with Visual C++ (CL)
#ifdef _MSC_VER
#define ATOM_MSVC
#endif

// How many threads are available or preferable? (This is currently unimplemented)
#define ATOM_MAX_THREADS 4
