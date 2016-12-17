#ifndef GEN_HPP
#define GEN_HPP
#include <type_traits>
namespace utils {

template <typename R>
class vgen_base {
	bool _state = false;
  	R storage;
public:
    R& get() {
    	return storage;
    }
    void set(R r) {
        storage = r;
    }
    void start() {
      	_state = true;
    }
  	void end() {
    	_state = false;
    }
    bool valid() const {
      	return _state;
    }
  	virtual vgen_base& operator++() = 0;
};

}
#endif
