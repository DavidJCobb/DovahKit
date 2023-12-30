#pragma once
#include <string>
#include <vector>
#include "./helpers/name_equals.h"
#include "./inheritance_status.h"
#include "./property.h"

namespace dovah::papyrus {
   class scriptobject {
      friend class scriptobject_list;
      protected:
         std::string           scriptname;
         inheritance_status    inheritance;
         std::vector<property> _properties;

      protected:
         constexpr const property* _get_property_by_name(const std::string_view&) const;
         constexpr property* _get_property_by_name(const std::string_view&);

         property* _add_property(const std::string_view& name);
         void _remove_property(const std::string_view& name);

         template<bool Const>
         class property_list_wrapper {
            protected:
               using owner_type = std::conditional_t<Const, const scriptobject, scriptobject>;

               owner_type& owner;

            protected:
               template<bool Const>
               class _iterator {
                  protected:
                     using owner_type = std::conditional_t<Const, const scriptobject, scriptobject>;

                     owner_type& owner;
                     size_t i = 0;

                     _iterator(owner_type& o, size_t i) : owner(o), i(i) {}

                  public:
                     const property& operator*() const { return this->owner._properties[i]; }
                     const property* operator->() const { return &this->owner._properties[i]; }
                     property& operator*() requires (!Const) { return this->owner._properties[i]; }
                     property* operator->() requires (!Const) { return &this->owner._properties[i]; }

                     constexpr _iterator& operator++() { return operator+=(1); }
                     constexpr _iterator& operator--() { return operator-=(1); }
                     constexpr _iterator  operator++(int) { return _iterator(*this) += 1; }
                     constexpr _iterator  operator--(int) { return _iterator(*this) -= 1; }
                     constexpr _iterator& operator+=(size_t i) { i += i; return *this; }
                     constexpr _iterator& operator-=(size_t i) { i -= i; return *this; }

                     friend constexpr std::partial_ordering operator<=>(const _iterator& a, const _iterator& b) {
                        return a.i <=> b.i;
                     }
                     constexpr bool operator==(const _iterator& other) const {
                        return (*this <=> other) == std::partial_ordering::equivalent;
                     }
               };

            public:
               using iterator = _iterator<false>;
               using const_iterator = _iterator<true>;

            public:
               constexpr property_list_wrapper(owner_type& o) : owner(o) {}

               constexpr property* add(const std::string_view& name) requires (!Const) { return this->owner._add_property(name); }
               constexpr property* remove(const std::string_view& name) requires (!Const) { return this->owner._remove_property(name); }

               constexpr const property* lookup(const std::string_view& name) const { return std::as_const(this->owner)._get_property_by_name(name); }
               constexpr property* lookup(const std::string_view& name) requires (!Const) { return this->owner._get_property_by_name(name); }

               constexpr size_t size() const noexcept { return this->owner._properties.size(); }

               iterator begin() requires (!Const) { return iterator(this->owner, 0); }
               iterator end() requires (!Const) { return iterator(this->owner, size()); }
               const_iterator cbegin() const { return const_iterator(this->owner, 0); }
               const_iterator cend() const { return const_iterator(this->owner, size()); }
               const_iterator begin() const { return const_iterator(this->owner, 0); }
               const_iterator end() const { return const_iterator(this->owner, size()); }
         };

      public:
         constexpr const std::string_view name() const { return this->scriptname; }

         constexpr bool name_matches(const std::string_view& v) const {
            return helpers::name_equals(v, this->scriptname);
         }
         constexpr bool name_matches(const std::string& v) const {
            return helpers::name_equals(v, this->scriptname);
         }

         constexpr const property_list_wrapper<true> properties() const { return property_list_wrapper<true>(*this); }
         constexpr property_list_wrapper<false> properties() { return property_list_wrapper<false>(*this); }

         constexpr bool is_inherited() const { return this->inheritance.present_on_base; }
         constexpr bool is_inherited_and_removed() const { return this->inheritance.removed_on_target; }
         constexpr bool is_locally_altered() const { return this->inheritance.present_on_target && !this->inheritance.removed_on_target; }
   };
}

#include "./scriptobject.inl"