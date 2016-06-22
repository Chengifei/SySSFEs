#include "quad_node.hpp"

namespace utils {

quad_node::~quad_node() noexcept {
    if (up) {
        //delete up;
        buffer.deallocate(up);
        up = nullptr;
    }
    left->right = right;
    right->left = left;
    if (down.tag)
        down->up = nullptr;
}

pool<quad_node, 10000> utils::quad_node::buffer;

}
