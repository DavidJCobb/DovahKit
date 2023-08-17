# String helpers
## Number parsing

Ordinarily, one would use standard library functions like `strtod` and `strtol` to parse string content into numbers. However, these have a few drawbacks:

* They're not `constexpr`, so you can't use `static_assert` to verify the correctness of any code that uses them.

* They depend on the current C locale, which is global non-thread-safe state that can be modified by any thread or library in the application at any time, including while any function dependent on it is running. The C locale and virtually every function relying on it is a footgun.

* The C locale is also just completely inappropriate for many kinds of "text" data. For example, if I have an INI file that sets some value to `1.3`, the effect of that configuration file should not change if I share it with a friend in Europe. (And sure, you could just force the current locale to `"C"` and *hope* nothing pulls that out from under you, but a non-constexpr house-of-cards ill-conceived-from-the-start legacy system seems best avoided, to me.)

Thus, some hand-written parsing functions.