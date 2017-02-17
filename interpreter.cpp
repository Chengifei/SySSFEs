/* Copyright 2017 by Yifei Zheng
 * This file is part of CSF (formerly SAP)
 * Unauthorized copy, modification or distribution is prohibited.
 */

#include "bytecodes.hpp"
#include "func.hpp"
#include <cmath>
#include "math/vector.hpp"
#include "math/common.hpp"
#include "reader.hpp"
#include <cstring>


void func::exec(shared& srd, Map::const_iterator begin, Map::const_iterator end) {
    auto it = begin;
    using namespace detail;
    while (it != end) {
        switch (*it++) {
            case PUSHM:
                {
                unsigned align = *it++;
                srd.push(srd.get_obj(read_as<unsigned>(it)) + read_as<unsigned>(it), align, align);
                }
                break;
            case PUSHV:
                {
                unsigned align = *it++;
                srd.push(srd.read(align, read_as<unsigned>(it)), align, align);
                }
                break;
            case PUSHI:
                {
                unsigned align = *it++;
                srd.push(&*it, align, align);
                it += align;
                }
                break;
            case POPM:
                {
                unsigned align = *it++;
                std::memcpy(srd.get_obj(read_as<unsigned>(it)) + read_as<unsigned>(it),
                            srd.pop(align, align), align);
                }
                break;
            case POPV:
                {
                unsigned align = *it++;
                srd.store(srd.pop(align, align), align, align);
                }
                break;
            case AEQ:
                srd.pushf(srd.pop<vector<double>>() == srd.pop<vector<double>>());
                break;
            case AEQV:
                srd.pushf(srd.pop<vector<double>>() == srd.pop<vector<double>>());
                break;
            case CMP:
                {
                auto val2 = srd.pop<double>();
                auto val1 = srd.pop<double>();
                srd.pushf((val2 > val1) - (val2 < val1));
                }
                break;
            case AND:
                srd.pushf(srd.popf() && srd.popf());
                break;
            case OR:
                srd.pushf(srd.popf() || srd.popf());
                break;
            case NOT:
                srd.pushf(!srd.popf());
                break;
            case ADD:
                srd.push(srd.pop<double>() + srd.pop<double>());
                break;
            case SUB:
                {
                auto subtraend = srd.pop<double>();
                srd.push(srd.pop<double>() - subtraend);
                }
                break;
            case NEG:
                srd.push(- srd.pop<double>());
                break;
            case MUL:
                srd.push(srd.pop<double>() * srd.pop<double>());
                break;
            case DIV:
                {
                auto divisor = srd.pop<double>();
                srd.push(srd.pop<double>() / divisor);
                }
                break;
            case ADV:
                srd.push(srd.pop<vector<double>>() + srd.pop<vector<double>>());
                break;
            case SBV:
                {
                auto subtraend = srd.pop<vector<double>>();
                srd.push(srd.pop<vector<double>>() - subtraend);
                }
                break;
            case MULV:
                srd.push(srd.pop<vector<double>>().mul(srd.pop<double>()));
                break;
            case DIVV:
                {
                auto divisor = srd.pop<double>();
                srd.push(srd.pop<vector<double>>() / divisor);
                }
                break;
            case CALL:
                switch (*it++) {
                    case SQRT:
                        srd.push(std::sqrt(srd.pop<double>()));
                        break;
                    case SIN:
                        srd.push(std::sin(srd.pop<double>()));
                        break;
                    case COS:
                        srd.push(std::cos(srd.pop<double>()));
                        break;
                    case TAN:
                        srd.push(std::tan(srd.pop<double>()));
                        break;
                    case ARCSIN:
                        srd.push(std::asin(srd.pop<double>()));
                        break;
                    case ARCCOS:
                        srd.push(std::acos(srd.pop<double>()));
                        break;
                    case ARCTAN:
                        srd.push(std::atan(srd.pop<double>()));
                        break;
                    case POW:
                        {
                        auto idx = srd.pop<double>();
                        srd.push(std::pow(srd.pop<double>(), idx));
                        }
                        break;
                    case LN:
                        srd.push(std::log(srd.pop<double>()));
                        break;
                    case INT:
                        srd.push(srd.pop<double>() * srd.peek<double>(1));
                        break;
                    case DIFF:
                        srd.push(srd.pop<double>() / srd.peek<double>(1));
                }
        }
    }
}
