#include "phycomath.h"
#ifdef PY_EXT
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#endif
#include <iostream>

bool is_alpha(string str) {
    static const char low = 'a';
    static const char LOW = 'A';
    static const char high = 'z';
    static const char HIGH = 'Z';
    for (char i : str) {
        if (!(i >= low && i <= high) || (i >= LOW && i <= HIGH))
            return false;
    }
    return true;
}

//There are strong evidence that shows manually written function are
//dramatically faster than it by regex.
int is_double(string str) {
    static const char low = '0';
    static const char high = '9';
    int mark = 0;
    bool ret = true;
    if (str.back() != '.') {
        for (char i : str) {
            if (low <= i && i <= high) {
            }
            else if (i == '.')
                mark++;
            else {
                ret = false;
                break;
            }
        }
    }
    if (mark == 1)
        return ret;
    else if (mark == 0 && ret)
        return 2;
    else
        return false;
}

namespace math {

auto Op::wrapper(unfunc func) {
    return [func](double argument, double) {return func(argument); };
}
Op::Op(string name, unsigned priority, binfunc func, bool infix) :
    name(name), priority(priority), infix(infix), func(func) {
}
Op::Op(string name, unsigned priority, unfunc func, bool infix) :
    name(name), priority(priority), infix(infix), func(Op::wrapper(func)) {
}
#ifdef PY_EXT
string Op::repr() const {
    return "Operator '" + this->name + "' with priority "
        + std::to_string(priority);
}
#endif

namespace operators {

const Opdict infix_dict{ { "+", PLU_OP },{ "-", MIN_OP },{ "*", MUL_OP },{
    "/", DIV_OP },{ "^", POW_OP } };

const Opdict built_in_dict{ { "sin", SIN_OP }  };

set<string> infixlist{ "(", ")" };
set<string> bilist{};

void init() {
    for (auto i : infix_dict) {
        infixlist.insert(i.first);
    }
    for (auto i : built_in_dict) {
        bilist.insert(i.first);
    }
}
}

plexpr::plexpr(Op op, arg_t arg1, arg_t arg2) :
    req(req_t::arg2_), op(op), arg1(arg1), arg2(arg2) {
    //		this->req.push(&(this->arg1));
    //		this->req.push(&(this->op));
    this->brackets.push(std::shared_ptr<plexpr>(this));
}
plexpr::plexpr(expr_ptr expr) :
    op(expr->op), arg1(expr->arg1), arg2(expr->arg2) {
}

bool plexpr::sequal(expr_ptr other) {
    return ((this->op.name == other->op.name) && other->arg1 == this->arg1) && this->arg2 == other->arg2;
}

namespace convert {

struct Arg2str_visitor : public boost::static_visitor<string> {
    string operator()(double arg) const {
        return std::to_string(arg);
    }
    string operator()(int arg) const {
        return std::to_string(arg);
    }
    string operator()(expr_ptr arg) const {
        //		return boost::apply_visitor(Arg2str_visitor(), arg->arg1) + arg->op.name
        //				+ boost::apply_visitor(Arg2str_visitor(), arg->arg2);
        return arg->op.name + "("
            + boost::apply_visitor(Arg2str_visitor(), arg->arg1) + ", "
            + boost::apply_visitor(Arg2str_visitor(), arg->arg2) + ")";
    }
    string operator()(string str) const {
        return str;
    }
    string operator()(decltype(nullptr)) const {
        return "_";
    }
};

struct simp2str_visitor : public boost::static_visitor<string> {
    template<class T>
    string operator()(T arg) const {
        return std::to_string(arg);
    }
    string operator()(string arg) const {
        return arg;
    }
    string operator()(plexpr arg) const {
        return expr2str(arg);
    }
};

inline string Arg2str(arg_t arg) {
    return boost::apply_visitor(Arg2str_visitor(), arg);
}

inline string expr2str(plexpr expr) {
    return Arg2str(std::make_shared<plexpr>(expr));
}
//inline string simp2str(simplify::simpr_t arg) {
//    return boost::apply_visitor(simp2str_visitor(), arg);
//}


struct Argexec_visitor : public boost::static_visitor<double> {
    double operator()(double arg) const {
        return arg;
    }
    double operator()(int arg) const {
        return arg;
    }
    double operator()(expr_ptr arg) const {
        try {
            return arg->op.func(
                boost::apply_visitor(Argexec_visitor(), arg->arg1),
                boost::apply_visitor(Argexec_visitor(), arg->arg2));
        }
        catch (std::runtime_error& e) {
            throw std::runtime_error(
                "Unexpected error at exec: " + string(e.what())
                + " with current expression: " + expr2str(*arg));
        }
    }
    double operator()(string) const {
        throw std::runtime_error(
            "Attempt to evaluate numeric result without full assignment");
    }
    double operator()(decltype(nullptr)) const {
        throw std::runtime_error("Attempt to dereference void* "
                                 "(HINT:Possibly incomplete expression)");
    }
};

}

/*
It seems this is a bad practice. Maybe will be improved later.
In the mean time, be sure to only use on very definite cases.
Also note the run-time error boost::bad_get only in append(Op&)
would be caught.
*/
#define B_R_A(X) (boost::get<arg_t*>(X)) // boost_req_t_Arg_ptr
#define B_R_O(X) (boost::get<Op*>(X)) // boost_req_t_Op_ptr
#define B_A_P(X) (boost::get<expr_ptr>(X)) // boost_arg_t_shared_ptr_plexpr

double plexpr::exec() {
    arg_t _ = std::make_shared<plexpr>(*this);
    return boost::apply_visitor(convert::Argexec_visitor(), _);
}

void plexpr::append(int arg) {
    switch (req) {
        case 2:
            brackets.top()->arg2 = arg;
            break;
        case 1:
            brackets.top()->arg1 = arg;
            break;
        default:
            throw std::runtime_error("");
    }
}

void plexpr::append(double arg) {
    switch (req) {
        case 2:
            brackets.top()->arg2 = arg;
            break;
        case 1:
            brackets.top()->arg1 = arg;
            break;
        default:
            throw std::runtime_error("");
    }
}

void plexpr::appendvar(string varname) {
    switch (req) {
        case 2:
            brackets.top()->arg2 = varname;
            break;
        case 1:
            brackets.top()->arg1 = varname;
            break;
        default:
            throw std::runtime_error("");
    }
}

void plexpr::append(Op& op) {
    /*this seemed to generate a lotta fragmentations (Anyone comes any idea? This code is
    enough to demonstrate the algorithm so I'm not axious to improve yet.)*/
    if (req) {
        if (op.priority <= brackets.top()->op.priority) {
            brackets.top()->arg1 = std::make_shared<plexpr>(brackets.top());
            brackets.top()->arg2 = nullptr;
            brackets.top()->op = op;
            req = req_t::arg2_;
        }
        else {
            brackets.top()->arg2 = std::make_shared<plexpr>(op,
                                                            brackets.top()->arg2, nullptr);
            brackets.push(B_A_P(brackets.top()->arg2));
            req = req_t::arg2_;
        }
    }
    else {
        brackets.top()->op = op;
        req = req_t::arg1_;
    }
}

void plexpr::append(string str) { //sort and branch
    if (str == "(")
        this->appendL();
    else if (str == ")")
        this->appendR();
    else {
        int i = is_double(str);
        if (i == 1)
            this->append(std::stod(str));
        else if (i == 2)
            this->append(std::stoi(str));
        else if (operators::infixlist.find(str) != operators::infixlist.end())
            this->append(const_cast<Op&>(operators::infix_dict.at(str)));
        else if (operators::bilist.find(str) != operators::bilist.end())
            this->appendBi(const_cast<Op&>(operators::built_in_dict.at(str)));
        else if (is_alpha(str))
            this->appendvar(str);
        else
            throw std::runtime_error(
                str + ": Not a valid variable name");
    }
}

void plexpr::appendL() {
    switch (req) {
        case 2:
            brackets.top()->arg2 = std::make_shared<plexpr>();
            brackets.push(B_A_P(brackets.top()->arg2));
            break;
        case 1:
            brackets.top()->arg1 = std::make_shared<plexpr>();
            brackets.push(B_A_P(brackets.top()->arg1));
            break;
        default:
            throw std::runtime_error("");
    }
    req=req_t::arg2_;
}

void plexpr::appendR() {
    this->brackets.pop();
}

void plexpr::appendBi(Op& op) {
    switch (req) {
        case 2:
            brackets.top()->arg2 = std::make_shared<plexpr>(op, nullptr, 0);
            brackets.push(B_A_P(brackets.top()->arg2));
            break;
        case 1:
            brackets.top()->arg1 = std::make_shared<plexpr>(op, nullptr, 0);
            brackets.push(B_A_P(brackets.top()->arg1));
            break;
        default:
            throw std::runtime_error("");
    }
    req = req_t::arg1_;
}

#undef B_R_A
#undef B_R_O
#undef B_A_P

plexpr parse(string str) {
    plexpr ret;
    for (auto i : process(str)) {
        ret.append(i);
    }
    return ret;
}

//namespace simplify {
//typedef boost::variant<plexpr, int, string, double> simpr_t;
//typedef std::function<bool(plexpr)> cond_t;
//typedef std::function<simpr_t(plexpr)> to_t;
//struct rewriting_rule {
//    cond_t cond;
//    to_t to;
//    rewriting_rule(cond_t cond, to_t to) :cond(cond), to(to) {
//
//    }
//    bool match(plexpr& expr) const {
//        return cond(expr);
//    }
//    simpr_t apply(plexpr expr) const {
//        auto _ = to(expr);
//        return _;
//    }
//
//};
//struct simp_rule :public rewriting_rule {
//
//};
//struct Arg_int_comp_visitor : public boost::static_visitor<bool> {
//    int i;
//    Arg_int_comp_visitor(int i) :i(i) {
//    }
//    template<class T>
//    bool operator()(T arg) const {
//        return false;
//    }
//    bool operator()(int arg) const {
//        return arg == i;
//    }
//};
//bool Arg_int_comp(arg_t arg, int i) {
//    return boost::apply_visitor(Arg_int_comp_visitor(i), arg);
//}
//#define COND(STR) [](plexpr _) {return STR;}
//#define TO(STR) [](plexpr _) {return STR;}
//const rewriting_rule PLU_C(COND(_.op.name == "+"), TO(plexpr(_.op, _.arg2, _.arg1)));
//const rewriting_rule MUL_C(COND(_.op.name == "*"), TO(plexpr(_.op, _.arg2, _.arg1)));
//const rewriting_rule MUL_0(COND((Arg_int_comp(_.arg1, 0) || Arg_int_comp(_.arg2, 0)) && _.op.name == "*"), TO(0));
//#undef COND
//#undef TO
//}

}
