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
* Leaf nodes do not contain a child list.
* In general, `constexpr` arrays need to be generated and used behind the scenes to match node data types to their attributes (e.g. whether a data type is a leaf). However, when an attribute is the same for all data types, we skip generating and using these arrays.

Even though nodes are not polymorphic, the use of metaprogramming allows the base class to offer the following functionality:

* Despite the lack of a virtual destructor, deleting a node via an untyped node pointer will still clean up subclass (typed node) members. This is because we use metaprogramming to allow the untyped node class to know (for any given data type) the offsets of its (templated) subclass's members.
* Similarly, because the base class knows the offset of its subclass's members, including the child list, you can manipulate the node hierarchy (i.e. add, remove, and access child nodes) using an untyped node pointer.

Unfortunately, the shenanigans needed to implement these optimzations make `cobb::node` *thoroughly* constexpr-incompatible.