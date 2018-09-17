#include "../Expr.hpp"
#include <cstring>
#include <test/framework.hpp>

using support::Expr;

#define STHISLINE "\n", __LINE__, ":", read_line{{}, __FILE__, __LINE__}
static void* make_ptr(int i) {
    return reinterpret_cast<void*>(i);
}

int main() {
    Expr expr(new Expr::Op(2, nullptr));
    expr.op->args[0].type = 0;
    expr.op->args[0].data = "1";
    expr.op->args[1] = Expr(new Expr::Op(2, make_ptr(1)));
    expr.op->args[1].op->args[0].type = 0;
    expr.op->args[1].op->args[0].data = "2";
    expr.op->args[1].op->args[1].type = 0;
    expr.op->args[1].op->args[1].data = "3";
    {
    support::Expr_preorder_iter iter(expr);
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, -1, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("1", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, -1, STHISLINE);
    EXPECT_EQ((*iter).op->args.size(), 2, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("2", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("3", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_TRUE(iter.exhausted(), STHISLINE);
    }
    {
    support::Expr_preorder_iter it(expr);
    unsigned cnt = 0;
    for (auto iter = it.begin(); iter != it.end(); ++iter)
        ++cnt;
    EXPECT_EQ(cnt, 5, STHISLINE);
    }
    {
    support::Expr_postorder_iter iter(expr);
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("1", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("2", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("3", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, -1, STHISLINE);
    EXPECT_EQ((*iter).op->op_data, make_ptr(1), STHISLINE);
    EXPECT_EQ((*iter).op->args.size(), 2, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, -1, STHISLINE);
    EXPECT_EQ((*iter).op->args.size(), 2, STHISLINE);
    ++iter;
    EXPECT_TRUE(iter.exhausted(), STHISLINE);
    }
    {
    support::Expr_postorder_iter it(expr);
    unsigned cnt = 0;
    for (auto iter = it.begin(); iter != it.end(); ++iter)
        ++cnt;
    EXPECT_EQ(cnt, 5, STHISLINE);
    }
    struct functor {
        bool& f;
        functor(bool& f) : f(f) {
            f = true;
        }
        ~functor() {
            f = false;
        }
    };
    {
    support::Expr_custom_iter<functor> iter(expr);
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, -1, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("1", (*iter).data, 1), 0, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, -1, STHISLINE);
    EXPECT_EQ((*iter).op->args.size(), 2, STHISLINE);
    bool flag = false;
    iter.emplace(flag);
    EXPECT_TRUE(flag, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("2", (*iter).data, 1), 0, STHISLINE);
    EXPECT_TRUE(flag, STHISLINE);
    ++iter;
    EXPECT_FALSE(iter.exhausted(), STHISLINE);
    EXPECT_EQ((*iter).type, 0, STHISLINE);
    EXPECT_EQ(std::strncmp("3", (*iter).data, 1), 0, STHISLINE);
    EXPECT_TRUE(flag, STHISLINE);
    ++iter;
    EXPECT_TRUE(iter.exhausted(), STHISLINE);
    EXPECT_FALSE(flag, STHISLINE);
    }
    std::cout << "All tests passed\n";
}
