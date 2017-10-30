//
// Created by Yifei Zheng on 08/09/2017.
//

#ifndef ATOM_CODEGEN_SUPPORT_TYPES_HPP
#define ATOM_CODEGEN_SUPPORT_TYPES_HPP
struct Types {
    enum : char {
        DOUBLE,
        INT,
        BUFFER
    } base;
    bool is_const;
    int agg;
};
#endif
