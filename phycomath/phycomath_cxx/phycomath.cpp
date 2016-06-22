/*! \file */
#include "phycomath.hpp"
#include "math_utils.cpp"
#include <stack>
#ifndef PHYCO_USE_UMAP
#define unordered_map map
#endif

namespace math {

const Op* get_op(const string& str) {
    switch (str2int(str.c_str())) {
        case str2int("+"):
            return &operators::PLU_OP;
        case str2int("-"):
            return &operators::MIN_OP;
        case str2int("*"):
            return &operators::MUL_OP;
        case str2int("/"):
            return &operators::DIV_OP;
        case str2int("^"):
            return &operators::POW_OP;
    }
}

struct arg_equal_visitor :public utils::variant::static_visitor<bool> {
    template <class T, class U>
    bool operator()(T a, U b) const {
        return false;
    }
    template <class T>
    bool operator()(T a, T b) const {
        return a == b;
    }
};

template<class cmp_t>
struct Arg_comp_visitor : public utils::variant::static_visitor<bool> {
private:
    cmp_t i;
public:
    Arg_comp_visitor(cmp_t i) :i(i) {}
    template<class T>
    bool operator()(T) const {
        return false;
    }
    bool operator()(cmp_t arg) const {
        return arg == i;
    }
};

Op::Op(string name, unsigned priority, binfunc func, bool infix) :
    name(name), priority(priority), minfix(infix), func(func) {}

#ifdef PY_EXT
string Op::repr() const noexcept {
    return "Operator '" + name + "' with priority "
        + std::to_string(priority);
}
#endif

namespace operators {

const Opdict built_in_dict{ { "sin", &SIN_OP }, {"cos", &COS_OP}, {"tan", &TAN_OP}, {"ln", &LN_OP} , {"exp", &EXP_OP}, {"sqrt", &SQRT_OP} };

std::set<string> infixlist{ "(", ")", "+", "-", "*", "/", "^" };
std::set<string> bilist{};

void init() noexcept {
    for (auto i : built_in_dict) {
        bilist.insert(i.first);
    }
}

} // namespace operators

namespace convert {

inline int stoi(string s) noexcept {
    int x = 0;
    for (auto i : s)
        x = (x * 10) + (i - '0');
    return x;
}

struct arg_to_str_visitor : public utils::variant::static_visitor<string> {
    string operator()(double arg) const {
        return std::to_string(arg);
    }
    string operator()(int arg) const {
        return std::to_string(arg);
    }
    string operator()(string str) const {
        return str;
    }
    string operator()(const Op* op) const {
        return op->name;
    }
};

}

class parse_manager {
    typedef expression_tree::node node;
    expression_tree tree;
    node* cursor = tree.mroot;
    std::vector<node*> vec{ cursor };
    std::stack<node*> stack;
    bool last_is_built_in = false;
    void append(int arg) noexcept {
        cursor->append(arg);
    }
    void append(double arg) noexcept {
        cursor->append(arg);
    }
    void append(string arg) noexcept {
        cursor->append(arg_t(arg));
    }
    void append(const Op* op) noexcept {
        if (cursor->content.o->priority > op->priority) {
            if (!cursor->parent) {
                cursor->parent = new(utils::allocator<node>::allocate(1)) node(arg_t(op), nullptr);
                vec.push_back(cursor->parent);
                cursor->parent->append(cursor);
                cursor = cursor->parent;
                tree.mroot = cursor;
            }
            else {
                cursor = cursor->parent;
                append(op);
            }
        }
        else if (cursor->content.o->priority == op->priority) {
            if (!(*cursor->content.o == *op)) {
                if (cursor->parent)
                    cursor->parent->args.pop_back();
                cursor->parent = new(utils::allocator<node>::allocate(1)) node(arg_t(op), cursor->parent);
                vec.push_back(cursor);
                cursor->parent->append(cursor);
                cursor = cursor->parent;
                if (cursor->parent)
                    cursor->parent->append(cursor);
                else
                    tree.mroot = cursor;
            }
        }
        else {
            auto t = cursor->args.back();
            cursor->args.pop_back();
            cursor->append(arg_t(op));
            cursor = cursor->args.back();
            vec.push_back(cursor);
            cursor->append(t);
        }
    }
    void appendL() noexcept {
        stack.push(cursor);
        cursor->append(arg_t(&operators::PLU_OP));
        cursor = cursor->args.back();
        vec.push_back(cursor);
        cursor->append(arg_t(0));
    }
    void appendR() noexcept {
        cursor = stack.top();
        stack.pop();
        if (operators::bilist.count(cursor->content.o->name))
            cursor = cursor->parent;
    }
    void append_built_in(const Op* op) noexcept {
        cursor->append(arg_t(op));
        cursor = cursor->args.back();
        vec.push_back(cursor);
    }
public:
    parse_manager() {
        cursor->content = &operators::PLU_OP;
        cursor->append(arg_t(0));
    }
    void append_branch(string str) {
        if (str[0] == '(') { // str[0] is the whole string, for performance
            appendL();
        }
        else if (str[0] == ')') {
            appendR();
        }
        else {
            int i = is_double(str);
            if (i == 1)
                append(std::stod(str));
            else if (i == 2)
                append(convert::stoi(str));
            else if (operators::infixlist.find(str) != operators::infixlist.end()) {
                append(get_op(str));
            }
            else if (operators::bilist.count(str)) {
                append_built_in(operators::built_in_dict.at(str));
            }
            else if (is_alpha(str))
                append(str);
            else
                throw std::runtime_error(
                    str + ": Not a valid variable name");
        }
    }
public:
    auto get_tree() noexcept {
        return std::move(tree);
    }
    auto get_vec() noexcept {
        return std::move(vec);
    }
};

expression_tree parse(string input, trie& Trie) {
    parse_manager a;
    for (string& i : process(input, Trie)) {
        a.append_branch(std::move(i));
    }
    return a.get_tree();
}

eqn_t::eqn_t(const expression_tree& lhs, const expression_tree& rhs) : lhs(lhs), rhs(rhs) {}

void eqn_t::group_to_left() {
    auto temp = lhs.mroot;
    lhs.mroot = new(utils::allocator<expression_tree::node>::allocate(1)) expression_tree::node(arg_t(&operators::MIN_OP), nullptr);
    lhs.mroot->append(temp);
    lhs.mroot->append(rhs.mroot);
    rhs.mroot = nullptr;
}

double eqn_t::fNsolve(string varname, double seed) {
    if (rhs.mroot)
        group_to_left();
    return solver::fNsolve(lhs, varname, seed, {});
}

namespace simplify {

template <class cmp_t>
inline bool Arg_comp(arg_t& arg, cmp_t i) {
    return utils::variant::apply_visitor(Arg_comp_visitor<cmp_t>(i), arg);
}

template <class T, class U>
inline bool cmp_second(std::pair<T, U> a, unsigned b) noexcept {
    return a.second > b;
}

std::vector<std::pair<expression_tree::node*, unsigned>>
find_leaves(std::vector<expression_tree::node*> v) {
    std::vector<std::pair<expression_tree::node*, unsigned>> ret;
    for (auto i : v) {
        ret.insert(std::lower_bound(ret.begin(), ret.end(), i->depth(), cmp_second<expression_tree::node*, unsigned>), { i,i->depth() });
    }
    return ret;
}

struct reduce_base {
    static void reduce(expression_tree::node*) {}
};

struct filter_only_plain {
    static bool test(expression_tree::node* ptr) {
        return ptr->content.tag == 1 || ptr->content.tag == 0;
    }
};

struct filter_only_add {
    static bool test(expression_tree::node* ptr) {
        return ptr->content.tag == 3 && ptr->content.o->name == "+";
    }
};

struct add_reduce : reduce_base {
    static void reduce(expression_tree::node* ptr) {
        auto non_plain = filter<std::vector<expression_tree::node*>, filter_only_plain>(ptr->args);
        auto cant_simplify = filter<std::vector<expression_tree::node*>, filter_only_add>(non_plain);
        // non_plain now reduce to add only
        double arg = 0;
        for (auto i : ptr->args) {
            arg += expression_tree::cast_to_double(i, {});
            utils::allocator<expression_tree::node>::deallocate(i);
        }
        ptr->args.clear();
        for (auto i : non_plain) {
            merge(i, ptr);
        }
        if (cant_simplify.empty() && non_plain.empty())
            ptr->content = arg;
        else if (!non_plain.empty()) {
            //non_plain.clear(); // no use
            ptr->append(arg);
            reduce(ptr);
        }
        else {
            ptr->args = cant_simplify;
            ptr->append(arg);
        }

    }
    static void merge(expression_tree::node* from, expression_tree::node* to) {
        from->args.reserve(from->args.size() + to->args.size());
        to->args.insert(to->args.end(), from->args.begin(), from->args.end());
    }
    template <class C>
    static void move(std::vector<C>& a, std::vector<C>&& b) {
        a.reserve(a.size() + b.size());
        a.insert(a.end(), b.begin(), b.end());
    }
};

struct mul_reduce : reduce_base {
    static void reduce(expression_tree::node* ptr) {
        auto non_plain = filter<std::vector<expression_tree::node*>, filter_only_plain>(ptr->args);
        double arg = expression_tree::cast_to_double(ptr->args[0], {});
        for (auto it = ++ptr->args.begin(); it != ptr->args.end();) {
            arg *= expression_tree::cast_to_double(*it, {});
            it = ptr->erase(it);
        }
        if (non_plain.empty())
            ptr->content = arg;
        else if (arg == 0) {
            ptr->content = 0;
        }
        else {
            ptr->args = non_plain;
            ptr->append(arg);
        }
    }
};

void simplify(expression_tree& expr, std::vector<expression_tree::node*> v) {
    for (auto i : find_leaves(v)) {
        try {
            i.first->content = expression_tree::exec_at(i.first);
        }
        catch (...) {
            if (i.first->content.o->name == "+")
                add_reduce::reduce(i.first);
            else if (i.first->content.o->name == "*")
                mul_reduce::reduce(i.first);
        }
    }
}
} // namespace simplify

namespace calculus {

#ifndef _MSC_VER
constexpr
#endif
inline double STEP() noexcept {
    return sqrt(pow(10, -15));
}

double fdiff(const expression_tree& expr, string varname, double x) {
    double step = STEP() * x;
    std::unordered_map<string, double>vardict{ {varname, x - step} };
    auto y1 = expr.fexec(vardict);
    vardict[varname] = x + step;
    auto y2 = expr.fexec(vardict);
    return (y2 - y1) / STEP() / 2;
}
#undef STEP
} // namespace calculus

namespace solver {
#define TOLERANCE 1e-6
#define MAX_ITER 10000
double fNsolve(const expression_tree& expr, string varname, double seed, std::unordered_map<string, double> vardict) {
    unsigned i = 0;
    vardict = { { varname, seed } };
    double& x = vardict[varname];
    while (fabs(expr.fexec(vardict)) > TOLERANCE && i != MAX_ITER) {
        x -= expr.fexec(vardict) / calculus::fdiff(expr, varname, x);
        ++i;
    }
    if (fabs(expr.fexec(vardict)) <= TOLERANCE)
        return x;
    throw std::runtime_error("Newton's method not converging (HINT: Try changing the seed)");
}

} // namespace solver

} // namespace math

decltype(math::expression_tree::node::args) math::expression_tree::__dummy_vector{};

std::ostream& operator<<(std::ostream& os, const math::arg_t& arg) {
    os << utils::variant::apply_visitor(math::convert::arg_to_str_visitor(), arg);
    return os;
}

std::ostream& operator<<(std::ostream& os, const math::expression_tree::node& node) {
    os << math::expression_tree::str_at(&node);
    return os;
}

std::ostream& operator<<(std::ostream& os, const math::eqn_t& eqn) {
    os << eqn.repr();
    return os;
}
