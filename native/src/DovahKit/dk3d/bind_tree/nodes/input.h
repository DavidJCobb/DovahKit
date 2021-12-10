#pragma once
#include "dk3d/bind_tree/node.h"
#include "dk3d/inputs/bound_input.h"
#include "dk3d/tools/_base.h"
#include "dk3d/tools/_options.h"

namespace DK3D::binds::nodes {
   class input : public node {
      public:
         static constexpr node_type my_type = node_type::input;
      public:
         input() : node(my_type) {}

         QString name;
         bool    is_modifier = false;
         inputs::bound_input mapping;
         //
         const tools::base*  tool = nullptr;
         tools::option_union params;

         #pragma region helper constructors for predefined binds
         template<typename T> requires tools::tool_has_options_member_type<T>
         input(const QString& n, const inputs::bound_input& bi, T* f, const typename T::options& o, bool m = false) : node(my_type), name(n), mapping(bi), tool(f), params(o), is_modifier(m) {}

         template<typename T> requires tools::tool_lacks_options_member_type<T>
         input(const QString& n, const inputs::bound_input& bi, T* f, bool m = false) : node(my_type), name(n), mapping(bi), tool(f), is_modifier(m) {}

         input(const QString& n, const inputs::bound_input& bi, std::nullptr_t f, bool m = false) : node(my_type), name(n), mapping(bi), tool(f), is_modifier(m) {}
         #pragma endregion

         virtual bool check_still_active(DK3DInputHandler&) const override;
         virtual bool is_valid_parent() const override { return this->is_modifier; }

      protected:
         virtual node* _clone_impl() const override;
   };
}