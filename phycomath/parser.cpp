/*
 This is the string and string-represented math expression processing module of Phyco.
 Copyright 2016 by Yifei Zheng
 All rights reserved.
 */
//#define PY_EXT
#ifdef PY_EXT
#warning Python extension interface are not fully supported and tested\
 There are known issue that the program cannot be linked on Windows (as tested on Win7 x64)
#warning If anyone here succeeded to link it to a usable pyd, please notify the author.
#warning It seems that it's because Python from executable installer are compiled \
with MSVC compiler, and MinGW compiler wouldn't have access to its dynamic-library
#endif
#define PHYCO_MATH
#include <unordered_set>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <stdexcept>
using std::string;
template<class T>
using set=std::unordered_set<T>;
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
	node* parent;
	char letter;
	bool isroot;
	bool isEnd;
	set<node*> children { };
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
	node* appendchar(char letter, bool isEnd = false) {
		if (this->findchild(letter)) {
			this->linkchild(this->findchild(letter));
			return this->findchild(letter);
		} else {
			node* _ = new node(this, letter, isEnd);
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

const node* root = new node();
node* current = const_cast<node*>(root);
node* from = const_cast<node*>(root);
string notfound;
#ifdef PY_EXT
void build(boost::python::list keywords) {
	node* current;
	for (unsigned i = 0; i < boost::python::len(keywords); i++) {
		current = const_cast<node*>(root);
		for (char letter : (string) boost::python::extract < string
				> (keywords[i])) {
			current = current->appendchar(letter);
		}
		current->markend();
	}
}
#endif
void build(std::vector<string> keywords) {
	node* current;
	for (auto i : keywords) {
		current = const_cast<node*>(root);
		for (auto letter : i) {
			current = current->appendchar(letter);
		}
		current->markend();
	}
}
void reset() {
	from = trie::current;
	current = const_cast<node*>(root);
	notfound.clear();
}
bool put_in(char letter) {
	from = current;
	if (current->findchild(letter)) {
		current = trie::current->findchild(letter);
		return true;
	} else {
		current = const_cast<node*>(root);
		return false;
	}
}
node* getnode(string str) {
	trie::reset();
	for (unsigned i = 0; i < str.size(); i++) {
		bool r = trie::put_in(str.at(i));
		if (r) {
			return nullptr;
		}
	}
	return trie::current;
}
}
;

std::vector<string> process(string input) {
	trie::reset();
	std::vector < string > ret;
	unsigned i = 0;
	while (i < input.size()) {
		if (!trie::notfound.empty())
			ret.push_back(trie::notfound);
		trie::reset();
		if (trie::put_in(input.at(i))) {
			if (trie::current->isend()) {
				ret.push_back(trie::current->repr());
			}
		} else {
			do {
				trie::notfound += input.at(i);
				i++;
			} while (i < input.size() and !trie::put_in(input.at(i)));
			continue;
		}
		i++;
	}
	if (!trie::notfound.empty())
		ret.push_back(trie::notfound);
	return ret;
}

#ifdef PY_EXT
void init(boost::python::list keywords) {
	trie::build(keywords);
	trie::reset();
}
#else
void init(std::vector<string> v) {
	trie::build(v);
	trie::reset();
}
#endif

string tostr(std::vector<string> vec) {
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

#ifdef PHYCO_MATH

bool isalpha(string str) {
	static const char low = 'a';
	static const char LOW = 'A';
	static const char high = 'z';
	static const char HIGH = 'Z';
	for (char i : str) {
		if ((i >= low and i <= high) or (i >= LOW and i <= HIGH))
			return true;
	}
	return false;
}

//There are strong evidence that shows manually written function are
//dramatically faster than it by regex.
int isdouble(string str) {
	static const char low = '0';
	static const char high = '9';
	int mark = 0;
	bool ret = true;
	if (str.back() != '.') {
		for (char i : str) {
			if (low <= i and i <= high) {
			} else {
				if (i == '.') {
					mark++;
				} else {
					ret = false;
					break;
				}
			}
		}
	}
	if (mark == 1) {
		return ret;
	} else {
		if (mark == 0 and ret) {
			return 2;
		} else
			return false;
	}
}

namespace math {

struct Op {
	typedef double (*binfunc)(double, double);
	string name;
	unsigned priority;
	bool infix;
	binfunc func;
	Op(string name, unsigned priority, binfunc func = nullptr,
			bool infix = true) :
			name(name), priority(priority), infix(infix), func(func) {
	}
#ifdef PY_EXT
	Op(PyFunctionObject func) {
	}
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

const Op PLU_OP("+", 1, &funcs::plus);
const Op MIN_OP("-", 1, &funcs::minus);
const Op MUL_OP("*", 2, &funcs::mul);
const Op DIV_OP("/", 2, &funcs::div);
const Op POW_OP("^", 3, &pow);

typedef std::unordered_map<string, Op> Opdict;

const Opdict opdict { { "+", PLU_OP }, { "-", MIN_OP }, { "*", MUL_OP }, { "/",
		DIV_OP }, { "^", POW_OP } };
}

//class bracketstack { // no guard against impaired brackets
//	const std::unordered_map<string, bool> bdict { { "(", 1 }, { ")", 0 } };
//	unsigned Lcount;
//	void append(string bracket) {
//		if (bracket == ")")
//			Lcount--;
//		else
//			Lcount++;
//	}
//public:
//	void process(string str) {
//	}
//};

class plexpr;
typedef boost::variant<double, int, string, plexpr*, decltype(nullptr)> Arg;
typedef boost::variant<Arg*, Op*> req_t;

struct plexpr {
	Op op;
	Arg arg1;
	Arg arg2;
	std::queue<req_t> reqque; //request queue
	plexpr(Op op = operators::PLU_OP, Arg arg1 = 0, Arg arg2 = nullptr) :
			op(op), arg1(arg1), arg2(arg2) {
		this->reqque.push(&(this->arg1));
		this->reqque.push(&(this->op));
		this->reqque.push(&(this->arg2));
	}
	;
	plexpr(plexpr* expr) :
			op(expr->op), arg1(expr->arg1), arg2(expr->arg2) {
	}
	double exec();
	void append(int arg);
	void append(double arg);
	void append(Op& op);
	void append(string str);
	void appendL();
	void appendR() {
	}
}
;

namespace convert {

struct get_req_ptr_visitor: public boost::static_visitor<req_t> {
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

struct Arg2str_visitor: public boost::static_visitor<string> {
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

string expr2str(plexpr*);

struct Argexec_visitor: public boost::static_visitor<double> {
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
		} catch (std::runtime_error& e) {
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
		throw std::runtime_error("Attempt to dereference void*");
	}
};

inline string Arg2str(Arg arg) {
	return boost::apply_visitor(Arg2str_visitor(), arg);
}

inline string expr2str(plexpr* expr) {
	return Arg2str(expr);
}

inline req_t get_req_ptr(Arg& arg, int i = 2) {
	return boost::apply_visitor(get_req_ptr_visitor(i), arg);
}

inline req_t get_req_ptr(Arg* arg, int i = 2) {
	return boost::apply_visitor(get_req_ptr_visitor(i), *arg);
}

}

//It seems this is a bad practice. Maybe will be improved later.
//In the mean time, be sure to only use on very definite cases.
//Also note the run-time error boost::bad_get.
#define B_R_A(X) (boost::get<Arg*>(X)) // boost_req_t_Arg_ptr
#define B_R_O(X) (boost::get<Op*>(X)) // boost_req_t_Op_ptr
#define B_A_P(X) (boost::get<plexpr*>(X)) // boost_Arg_plexpr_ptr

double plexpr::exec() {
	Arg _ = this;
	return boost::apply_visitor(convert::Argexec_visitor(), _);
}

void plexpr::append(int arg) {
	*B_R_A(this->reqque.front()) = arg;
	this->reqque.pop();
}

void plexpr::append(double arg) {
	*B_R_A(this->reqque.front()) = arg;
	this->reqque.pop();
}

void plexpr::append(Op& op) {
	if (this->reqque.empty()) {
		if (op.priority <= this->op.priority) {
			this->arg1 = new plexpr(this);
			this->arg2 = nullptr;
			this->op = op;
			this->reqque.push(&(this->arg2));
		} else {
			this->arg2 = new plexpr(op, this->arg2, nullptr);
			this->reqque.push(convert::get_req_ptr(this->arg2));
		}
	} else {
		*B_R_O(this->reqque.front()) = op;
		this->reqque.pop();
	}
}

void plexpr::append(string str) { //sort and branch
	if (str == "(")
		this->appendL();
	else {
		if (str == "]")
			this->appendR();
		else {
			int i = isdouble(str);
			if (i == 1)
				this->append(std::stod(str));
			else {
				if (i == 2)
					this->append(std::stoi(str));
				else
					this->append(const_cast<Op&>(operators::opdict.at(str)));
			}
		}
	}
}

void plexpr::appendL() {
	Arg* _ = B_R_A(this->reqque.front());
	*_ = new plexpr();
	std::cout << convert::expr2str(this) << std::endl;
	this->reqque.push(convert::get_req_ptr(B_R_A(this->reqque.front()), 1));
	this->reqque.push(convert::get_req_ptr(B_R_A(this->reqque.front()), 0));
	this->reqque.push(convert::get_req_ptr(B_R_A(this->reqque.front())));
	this->reqque.pop();
}

#undef B_R_A
#undef B_R_O
#undef B_A_P

plexpr parse(string str) {
	plexpr ret { };
	for (auto i : process(str)) {
		std::cout << convert::expr2str(&ret) << std::endl;
		ret.append(i);
	}
	return ret;

}
}

#ifdef PY_EXT
BOOST_PYTHON_MODULE(parser)
{
	namespace py=boost::python;
	py::class_<std::vector<string> >("")
	.def(py::vector_indexing_suite<std::vector<string> >())
	.def("__repr__",&tostr)
	;

	py::class_<math::Arg>("arg",py::init<double>())
	.def(py::init<int>())
	.def("__repr__",&math::convert::Arg2str);

//	py::class_<math::Op>("Op",py::no_init)
	py::class_<math::Op>("op",py::init<PyFunctionObject>())
	.def("__repr__",&math::Op::repr);

	py::class_<math::plexpr>("plexpr",py::init<math::Op, math::Arg, math::Arg>())
	.def("exec",&math::plexpr::exec)
	.def("__repr__",&math::convert::expr2str);

	py::def("parse", process);

	py::def("mparse",math::parse);

	void (*initpy)(py::list)=&init;
	py::def("init", initpy);

}
#else
//#define EXEC_FAIL
int main(int argc, char* argv[]) {
	if (argc != 2)
		throw std::runtime_error("No value given to parse.");
	std::vector < string > oplist { "(", ")" };
	for (auto i : math::operators::opdict) {
		oplist.push_back(i.first);
	}
	::init (oplist);
	math::plexpr* _ = new math::plexpr;
	*_ = math::parse(argv[1]);
#ifndef EXEC_FAIL
	std::cout << math::convert::expr2str(_) << " = " << _->exec() << std::endl;
#else
	std::cout << math::convert::expr2str(_) << std::endl;
#endif
	return 0;
}
#endif
#else
int main() {
	return 0;
}
#endif
