/*
 This is the string and string-represented math expression processing module of Phyco.
 Copyright 2016 by Yifei Zheng
 All rights reserved.
 */
#ifdef _MSC_VER
#define BOOST_PYTHON_STATIC_LIB
#endif
 //#define PY_EXT
#define PHYCO_MATH
#if defined __GNUC__ && defined PY_EXT
#warning Python extension interface are not fully supported and tested\
There are known issue that the program cannot be linked on Windows(as tested on Win7 x64)
#warning If anyone here succeeded to link it to a usable pyd, please notify the author.
#warning It seems that it's because Python from executable installer are compiled \
with MSVC compiler, and MinGW compiler wouldn't have access to its dynamic-library.
#endif
#include <unordered_set>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#ifdef PHYCO_MATH
#include <queue>
#include <stack>
#include <cmath>
#include <functional>
#endif
using std::string;
template<class T>
using set = std::unordered_set<T>;
#include <boost/variant.hpp>
#ifdef PY_EXT
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#else
#include <iostream>
#ifdef PHYCO_DEBUG
#include <cassert>
#include <typeinfo>
#include <cxxabi.h>
#endif
#endif

class node {
#ifdef PHYCO_DEBUG
public:
#endif
    node* parent;
    char letter;
    bool isroot;
    bool isEnd;
    set<node*> children{};
public:
    string repr() const {
        if (parent)
            return this->parent->repr() + string(1, this->letter);
        else
            return "";
    }
    bool isend() const {
        return this->isEnd;
    }
    node* appendchar(char letter) {
        if (this->findchild(letter)) {
            this->linkchild(this->findchild(letter));
            return this->findchild(letter);
        }
        else {
            node* _ = new node(this, letter);
            this->linkchild(_);
            return _;
        }
    }
    node() :
        parent(nullptr), letter('\0'), isroot(true), isEnd(false) {
    }
    node(node* parent, char letter, bool isEnd = false, bool isroot = false) :
        parent(parent), letter(letter), isroot(isroot), isEnd(isEnd) {
    }
    node* findchild(char letter) const {
        for (auto i : this->children) {
            if (i->letter == letter) {
                return i;
            }
        }
        return nullptr;
    }
    bool haschild() const {
        return !this->children.empty();
    }
    //	node* operator --() {
    //		return this->parent;
    //	}
    void linkchild(node* child) {
        this->children.insert(child);
    }
    void markend() {
        this->isEnd = true;
    }
};

namespace trie {

node* const root = new node();
node* current = root;
node* from = root;
string notfound;

void build(set<string> keywords) {
    node* current;
    for (auto i : keywords) {
        current = root;
        for (auto letter : i) {
            current = current->appendchar(letter);
        }
        current->markend();
    }
}

void reset() {
    from = trie::current;
    current = root;
    notfound.clear();
}
bool put_in(char letter) {
    from = current;
    if (current->findchild(letter)) {
        current = current->findchild(letter);
        return true;
    }
    else {
        current = root;
        return false;
    }
}
node* getnode(string str) {
    trie::reset();
    for (unsigned i = 0; i < str.size(); i++) {
        if (!trie::put_in(str.at(i)))
            return nullptr;
    }
    return trie::current;
}

}

std::vector<string> process(string input) {
    trie::reset();
    std::vector < string > ret;
    unsigned i = 0;
    while (i < input.size()) {
        if (!trie::notfound.empty()) {
            ret.push_back(trie::notfound);
            trie::reset();
        }
        if (trie::put_in(input.at(i))) {
            if (trie::current->isend()) {
                ret.push_back(trie::current->repr());
                trie::reset();
            }
        }
        else {
            do {
                trie::notfound += input.at(i);
                i++;
            } while (i < input.size() && !trie::put_in(input.at(i)));
            continue;
        }
        i++;
    }
    if (!trie::notfound.empty())
        ret.push_back(trie::notfound);
    return ret;
}

#ifdef PY_EXT
string strvec_repr(std::vector<string> vec) {
    string ret = "[";
    if (!vec.empty()) {
        for (auto i : vec) {
            ret += "'" + i + "', ";
        }
        ret.resize(ret.size() - 2);
    }
    ret += "]";
    return ret;
}
#endif

#ifdef PHYCO_MATH

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

struct Op {
    typedef std::function<double(double, double)> binfunc;
    typedef std::function<double(double)> unfunc;
private:
    auto wrapper(unfunc func) {
        return [func](double argument, double) {return func(argument); };
    }
public:
    string name;
    unsigned priority;
    bool infix;
    binfunc func;
    Op(string name, unsigned priority, binfunc func, bool infix = true) :
        name(name), priority(priority), infix(infix), func(func) {
    }
    Op(string name, unsigned priority, unfunc func, bool infix = true) :
        name(name), priority(priority), infix(infix), func(Op::wrapper(func)) {
    }
#ifdef PY_EXT
    string repr() const {
        return "Operator '" + this->name + "' with priority "
            + std::to_string(priority);
    }
#endif
};

namespace operators {
namespace funcs {

double plus(double a, double b) {
    return a + b;
}
double minus(double a, double b) {
    return a - b;
}
double mul(double a, double b) {
    return a * b;
}
double div(double a, double b) {
    return a / b;
}

}

const Op PLU_OP("+", 1, (Op::binfunc) &funcs::plus);
const Op MIN_OP("-", 1, (Op::binfunc) &funcs::minus);
const Op MUL_OP("*", 2, (Op::binfunc) &funcs::mul);
const Op DIV_OP("/", 2, (Op::binfunc) &funcs::div);
#ifdef _MSC_VER
const Op POW_OP("^", 3, (Op::binfunc) &std::powl);
const Op SIN_OP("sin", 4, (Op::unfunc) &std::sinl, false);
#else
const Op POW_OP("^", 3, (Op::binfunc) &pow);
const Op SIN_OP("sin", 4, (Op::unfunc) &sin);
#endif

typedef std::unordered_map<string, Op> Opdict;

const Opdict infix__dict{ { "+", PLU_OP }, { "-", MIN_OP }, { "*", MUL_OP }, {
        "/", DIV_OP }, { "^", POW_OP } };

const Opdict built_in_dict{ /*{ "sin", SIN_OP } */ };

set<string> infixlist{ "(", ")" };
set<string> bilist{};

void init() {
    for (auto i : infix__dict) {
        infixlist.insert(i.first);
    }
    for (auto i : built_in_dict) {
        bilist.insert(i.first);
    }
}

}

struct plexpr;
typedef boost::variant<double, int, string, plexpr*, decltype(nullptr)> arg_t;
typedef boost::variant<arg_t*, Op*> req_t;

struct plexpr {
    Op op;
    arg_t arg1;
    arg_t arg2;
    std::pair<plexpr*, unsigned> bottom;
    std::queue<req_t> reqs; //request queue
    std::stack<plexpr*> brackets;
    plexpr(Op op = operators::PLU_OP, arg_t arg1 = 0, arg_t arg2 = nullptr) :
        op(op), arg1(arg1), arg2(arg2), bottom({ this,0 }) {
        //		this->reqs.push(&(this->arg1));
        //		this->reqs.push(&(this->op));
        this->reqs.push(&(this->arg2));
        this->brackets.push(this);
    }
    ;
    plexpr(plexpr* expr) :
        op(expr->op), arg1(expr->arg1), arg2(expr->arg2) {
    }
    double exec();
    void append(int);
    void append(double);
    void append(Op&);
    void append(string);
    void appendL();
    void appendR();
    void appendBi(Op&);
    void appendvar(string);
    bool sequal(plexpr* other) {
        return ((this->op.name == other->op.name) && other->arg1 == this->arg1) && this->arg2 == other->arg2;
    }
}
;

namespace convert {

struct get_req_ptr_visitor : public boost::static_visitor<req_t> {
    int i;
    get_req_ptr_visitor(int i) :
        i(i) {
    }
    template<class T>
    req_t operator()(T arg) const {
#ifdef PHYCO_DEBUG
        std::cout
            << abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr,
                                   nullptr) << std::endl;
#endif
        throw std::runtime_error("In get_req_ptr: Cast to plexpr not allowed");
    }
    req_t operator()(plexpr* arg) const {
        switch (this->i) {
            case 0:
                return &(arg->op);
            case 1:
                return &(arg->arg1);
            case 2:
                return &(arg->arg2);
            default:
                throw std::invalid_argument("");
        }
    }
};

string expr2str(plexpr*);

struct Argexec_visitor : public boost::static_visitor<double> {
    double operator()(double arg) const {
        return arg;
    }
    double operator()(int arg) const {
        return arg;
    }
    double operator()(plexpr* arg) const {
        try {
            return arg->op.func(
                boost::apply_visitor(Argexec_visitor(), arg->arg1),
                boost::apply_visitor(Argexec_visitor(), arg->arg2));
        }
        catch (std::runtime_error& e) {
            throw std::runtime_error(
                "Unexpected error at exec: " + string(e.what())
                + " with current expression: " + expr2str(arg));
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

inline req_t get_req_ptr(arg_t& arg, int i = 2) {
    return boost::apply_visitor(get_req_ptr_visitor(i), arg);
}

inline req_t get_req_ptr(arg_t* arg, int i = 2) {
    return boost::apply_visitor(get_req_ptr_visitor(i), *arg);
}

}

/*
It seems this is a bad practice. Maybe will be improved later.
In the mean time, be sure to only use on very definite cases.
Also note the run-time error boost::bad_get only in append(Op&)
would be caught.
*/
#define B_R_A(X) (boost::get<arg_t*>(X)) // boost_req_t_Arg_ptr
#define B_R_O(X) (boost::get<Op*>(X)) // boost_req_t_Op_ptr
#define B_A_P(X) (boost::get<plexpr*>(X)) // boost_Arg_plexpr_ptr

double plexpr::exec() {
    arg_t _ = this;
    return boost::apply_visitor(convert::Argexec_visitor(), _);
}

void plexpr::append(int arg) {
    *B_R_A(this->reqs.front()) = arg;
    this->reqs.pop();
}

void plexpr::append(double arg) {
    *B_R_A(this->reqs.front()) = arg;
    this->reqs.pop();
}

void plexpr::appendvar(string varname) {
    *B_R_A(this->reqs.front()) = varname;
    this->reqs.pop();
}

void plexpr::append(Op& op) {
    /*this seemed to generate a lotta fragmentations (Anyone comes any idea? This code is
    enough to demonstrate the algorithm so I'm not axious to improve yet.)*/
    if (this->reqs.empty()) {
        if (op.priority <= this->brackets.top()->op.priority) {
            this->brackets.top()->arg1 = new plexpr(this->brackets.top());
            this->brackets.top()->arg2 = nullptr;
            this->brackets.top()->op = op;
            this->reqs.push(&(this->brackets.top()->arg2));
        }
        else {
            this->brackets.top()->arg2 = new plexpr(op,
                                                    this->brackets.top()->arg2, nullptr);
            this->reqs.push(convert::get_req_ptr(this->brackets.top()->arg2));
        }
    }
    else {
        try {
            *B_R_O(this->reqs.front()) = op;
            this->reqs.pop();
        }
        catch (boost::bad_get) {
            throw std::runtime_error(
                "Internal error: Inconsistent type for appending an operator");
        }
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
            this->append(const_cast<Op&>(operators::infix__dict.at(str)));
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
    *B_R_A(this->reqs.front()) = new plexpr();
    this->brackets.push(
        const_cast<plexpr*>(B_A_P(*B_R_A(this->reqs.front()))));
    this->bottom.first = const_cast<plexpr*>(B_A_P(*B_R_A(this->reqs.front())));
    this->bottom.second++;
    this->reqs.push(convert::get_req_ptr(B_R_A(this->reqs.front())));
    this->reqs.pop();
}

void plexpr::appendR() {
    this->brackets.pop();
}

void plexpr::appendBi(Op& op) {
    *B_R_A(this->reqs.front()) = new plexpr(op, nullptr, 0);
    this->brackets.push(
        const_cast<plexpr*>(B_A_P(*B_R_A(this->reqs.front()))));
    this->bottom.first = const_cast<plexpr*>(B_A_P(*B_R_A(this->reqs.front())));
    this->bottom.second++;
    this->reqs.push(convert::get_req_ptr(B_R_A(this->reqs.front()), 1));
    this->reqs.pop();
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

namespace simplify {
typedef boost::variant<plexpr, int, string, double> simpr_t;
typedef std::function<bool(plexpr)> cond_t;
typedef std::function<simpr_t(plexpr)> to_t;
struct rewriting_rule {
    cond_t cond;
    to_t to;
    rewriting_rule(cond_t cond, to_t to) :cond(cond), to(to) {

    }
    bool match(plexpr& expr) const {
        return cond(expr);
    }
    simpr_t apply(plexpr expr) const {
        auto _ = to(expr);
        return _;
    }

};
struct simp_rule :public rewriting_rule {

};
struct Arg_int_comp_visitor : public boost::static_visitor<bool> {
    int i;
    Arg_int_comp_visitor(int i) :i(i) {
    }
    template<class T>
    bool operator()(T arg) const {
        return false;
    }
    bool operator()(int arg) const {
        return arg == i;
    }
};
bool Arg_int_comp(arg_t arg, int i) {
    return boost::apply_visitor(Arg_int_comp_visitor(i), arg);
}
#define COND(STR) [](plexpr _) {return STR;}
#define TO(STR) [](plexpr _) {return STR;}
const rewriting_rule PLU_C(COND(_.op.name == "+"), TO(plexpr(_.op, _.arg2, _.arg1)));
const rewriting_rule MUL_C(COND(_.op.name == "*"), TO(plexpr(_.op, _.arg2, _.arg1)));
const rewriting_rule MUL_0(COND((Arg_int_comp(_.arg1, 0) || Arg_int_comp(_.arg2, 0)) && _.op.name == "*"), TO(0));
#undef COND
#undef TO
}
namespace convert {
struct Arg2str_visitor : public boost::static_visitor<string> {
    string operator()(double arg) const {
        return std::to_string(arg);
    }
    string operator()(int arg) const {
        return std::to_string(arg);
    }
    string operator()(plexpr* arg) const {
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
        return expr2str(&arg);
    }
};

inline string Arg2str(arg_t arg) {
    return boost::apply_visitor(Arg2str_visitor(), arg);
}

inline string expr2str(plexpr* expr) {
    return Arg2str(expr);
}
inline string simp2str(simplify::simpr_t arg) {
    return boost::apply_visitor(simp2str_visitor(), arg);
}
}
}

void init() {
    math::operators::init();
    auto keywords = math::operators::infixlist;
    keywords.insert(math::operators::bilist.begin(),
                    math::operators::bilist.end());
    trie::build(keywords);
    trie::reset();
}

#ifdef PY_EXT
BOOST_PYTHON_MODULE(phycomath)
{
    namespace py = boost::python;
    py::class_<std::vector<string> >("std::vector<string>", py::no_init)
        .def(py::vector_indexing_suite<std::vector<string> >())
        .def("__repr__", &strvec_repr);

    py::class_<math::arg_t>("arg", py::init<double>())
        .def(py::init<int>())
        .def("__repr__", &math::convert::Arg2str);

    py::class_<math::Op>("Op", py::no_init)
        .def("__repr__", &math::Op::repr);

    py::class_<math::plexpr>("plexpr", py::init<math::Op, math::arg_t, math::arg_t>())
        .def("exec", &math::plexpr::exec)
        .def("__repr__", &math::convert::expr2str);

    py::def("parse", process);

    py::def("mparse", math::parse);

    void(*initpy)(void) = &init;
    py::def("init", initpy);

}
#else
int main(int argc, char* argv[]) {
    ::init();
    auto _ = new math::plexpr(math::operators::MUL_OP, 1, 1);
    std::cout << math::convert::expr2str(_) << "=";
    auto __ = math::simplify::MUL_0.apply(*_);
    std::cout << math::convert::simp2str(__) << std::endl;
    return 0;
}
#endif
#else
int main() {
    return 0;
}
#endif
