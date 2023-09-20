# `node` and `typed_node`

A helper class for node trees wherein nodes are treated as akin to containers.

The `node` class should be templated on the data types you wish to be able to store:

```c++
struct foo {
   int data;
};
struct bar {
   float data;
};

using my_node_type = cobb::node<foo, bar>;

auto* my_node_instance = my_node_type::make<foo>();
// decltype(my_node_instance)       is cobb::typed_node<my_node_type, foo>
// decltype(my_node_instance->data) is foo
```

You can also specify attributes on some data types:

```c++
using my_node_type = cobb::node<
   int,
   cobb::node_data_with_attributes<float, cobb::node_data_attribute::leaf>
>;
```

The `cobb::node` class represents an untyped node: we know what data types *can* be present, but the type tells us nothing about which data types *are* present in any given node. The templated `cobb::typed_node` subclass represents a typed node: its template parameters are a `cobb::node` specialization (i.e. which data types can be present) and a data type (i.e. which data type is present).

The following optimizations exist:

* The `node` class uses metaprogramming rather than polymorphism; the class is not `virtual` and instances therefore do not require a vtbl.
* In general, `constexpr` arrays need to be generated and used behind the scenes to match node data types to their attributes (e.g. whether a data type is a leaf). However, when an attribute is the same for all data types, we skip generating and using these arrays.

Even though nodes are not polymorphic, the use of metaprogramming allows the base class to offer the following functionality:

* Despite the lack of a virtual destructor, deleting a node via an untyped node pointer will still clean up subclass (typed node) members. This is because we use metaprogramming to allow the untyped node class to know (for any given data type) the offsets of its (templated) subclass's members.

Unfortunately, `cobb::node` is constexpr-incompatible due to both not being polymorphic and to the aforementioned shenanigans.

A saner approach would involve using a `std::variant` to hold the node contents, thereby avoiding the pitfalls of the above implementation and maintaining `constexpr` compatibility. However, that would also require that all of the types available in a given node tree be fully defined (not merely declared) in order to be able to access the node tree at all. With the current approach, you can merely forward-declare the data types present in the node tree and then be able to store instances of that node tree (e.g. as class members), reducing header dependencies; you only need to include the data type headers in full when you actually wish to operate on the node tree.