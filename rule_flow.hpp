#ifndef RULE_FLOW_HPP
#define RULE_FLOW_HPP
#include <vector>
#include <type_traits>
#include <functional>

template <typename shared_data_t, typename private_data_t>
class state {
    typedef typename std::add_lvalue_reference<shared_data_t>::type shared_reference;
    typedef typename std::add_lvalue_reference<private_data_t>::type private_reference;
public:
    private_data_t private_data;
    shared_reference shared_data;
    state(shared_reference sd, private_data_t pd)
        : private_data(pd), shared_data(sd) {}
    state* advance() {
        for (auto& i : children)
            if (i.private_data.enable(shared_data))
                return &i;
        return nullptr;
    }
    state* add_child_state(private_data_t&& pd) {
        children.emplace_back(shared_data, std::move(pd));
        return &children.back();
    }
private:
    std::vector<state> children;
};
#endif
