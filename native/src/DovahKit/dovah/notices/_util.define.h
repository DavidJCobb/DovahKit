#pragma push_macro("MAKE_CLONE_OVERLOAD")
#define MAKE_CLONE_OVERLOAD virtual base_warning* clone() const override { return new std::decay_t<decltype(*this)>{*this}; }

#pragma push_macro("MAKE_ERROR_OVERLOADS")
#define MAKE_ERROR_OVERLOADS virtual base_error* clone() const override { return new std::decay_t<decltype(*this)>{*this}; }