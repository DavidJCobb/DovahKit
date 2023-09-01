
If at some point we want to extend `cobb::pow` to support real-number exponents, we can take advantage of [the following identity](https://math.stackexchange.com/questions/132703/what-does-2x-really-mean-when-x-is-not-an-integer/133238#133238):

> For any <var>c</var>:  
> b<sup>x</sup> = c<sup>x * log<sub>c</sub>(b)</sup>

A common choice of <var>c</var> is 2 i.e. log<sub>2</sub>. In that case, some identities hold:

> log<sub>2</sub>(x * y) = log<sub>2</sub>(x) + log<sub>2</sub>(y)  
> &therefore; log<sub>2</sub>(-n) = log<sub>2</sub>(n) - log<sub>2</sub>(1)  
> log<sub>2</sub>(x / y) = log<sub>2</sub>(x) - log<sub>2</sub>(y)

If we can implement constexpr log<sub>2</sub> for any real, then we can implement constexpr exponentiation by any real. The `to_ratio.h` code will help with this: if we can convert our desired exponent into a ratio, then that's our <var>x</var> and <var>y</var> to stuff into the log<sub>2</sub>s above.