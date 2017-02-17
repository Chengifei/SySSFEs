/* Copyright 2017 by Yifei Zheng
 * This file is part of CSF (formerly SAP)
 * Unauthorized copy, modification or distribution is prohibited.
 *
 * These are bytecodes used in CSF.
 */
#ifndef BYTECODES_HPP
#define BYTECODES_HPP
enum instruction : char {
    NOP = 0x00,
    PUSHM = 0x01, // push member
    PUSHV = 0x02, // push global
    PUSHI = 0x03, // push immediate
    POPM = 0x04,
    POPV = 0x05,
    VRF = 0x06, // pop from flags and raise exception if it's not true
    CALL = 0x07,
    AEQ = 0x10,
    AEQV = 0x11,
    CMP = 0x12,
    COND = 0x13,
    AND = 0x14,
    OR = 0x15,
    NOT = 0x16,
    ADD = 0x17,
    SUB = 0x18,
    MUL = 0x19,
    DIV = 0x1a,
    NEG = 0x1b,
    ADV = 0x20,
    SBV = 0x21,
    MULV = 0x22,
    DIVV = 0x23,
    NEGV = 0x24,
    SOVSS = 0x25,
    SOVPS = 0x26,
    SOVSV = 0x27,
    SOVPV = 0x28,
};

enum func_code : char {
    SQRT = 0x00,
    SIN = 0x01,
    COS = 0x02,
    TAN = 0x03,
    ARCSIN = 0x04,
    ARCCOS = 0x05,
    ARCTAN = 0x06,
    POW = 0x07,
    LN = 0x08,
    INT = 0x09,
    DIFF = 0x0a
};
#endif
