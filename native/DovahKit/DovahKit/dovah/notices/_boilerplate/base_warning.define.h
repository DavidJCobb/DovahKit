#pragma push_macro("DEFINE_NOTICE_BOILERPLATE")
#undef  DEFINE_NOTICE_BOILERPLATE
#define DEFINE_NOTICE_BOILERPLATE virtual base_warning* clone() const override { return new std::decay_t<decltype(*this)>{*this}; }