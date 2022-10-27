#include <array>
#include <variant>
#include <vector>
#include "../../unreachable.h"

namespace cobb::impl::_euler_matrix {
   enum class trigonometric_function {
      none,
      cos,
      sin,
      tan,
   };
   struct basic_trigonometric_term {
      char   angle    = 'x';
      double constant = 0;     // ignored if angle is set
      bool   negate   = false; // used only if angle is set
      trigonometric_function trig = trigonometric_function::none;

      constexpr basic_trigonometric_term() {}
      constexpr basic_trigonometric_term(int c) : constant(c) {}
      constexpr basic_trigonometric_term(double c) : constant(c) {}
      constexpr basic_trigonometric_term(const char* str) {
         if (str[0] == '-') {
            negate = true;
            ++str;
         }
         if (str[2] == '\0') {
            angle = str[1];
            return;
         }

         if (str[3] != '(' && str[5] != ')')
            cobb::unreachable();
         if (str[0] == 'c' && str[1] == 'o' && str[2] == 's')
            trig = trigonometric_function::cos;
         else if (str[0] == 's' && str[1] == 'i' && str[2] == 'n')
            trig = trigonometric_function::sin;
         else if (str[0] == 't' && str[1] == 'a' && str[2] == 'n')
            trig = trigonometric_function::tan;
         angle = str[4];
      }
      template<size_t N> constexpr basic_trigonometric_term(const char str[N]) : basic_trigonometric_term(&str[0]) {}

      basic_trigonometric_term(const basic_trigonometric_term&) = default;
      basic_trigonometric_term(basic_trigonometric_term&&) noexcept = default;
      basic_trigonometric_term& operator=(const basic_trigonometric_term&) = default;
      basic_trigonometric_term& operator=(basic_trigonometric_term&&) = default;
   };

   class basic_euler_matrix {
      public:
         constexpr basic_euler_matrix() {}

         constexpr basic_euler_matrix(
            basic_trigonometric_term a,
            basic_trigonometric_term b,
            basic_trigonometric_term c,
            basic_trigonometric_term d,
            basic_trigonometric_term e,
            basic_trigonometric_term f,
            basic_trigonometric_term g,
            basic_trigonometric_term h,
            basic_trigonometric_term i
         ) : items({a,b,c,d,e,f,g,h,i}) {
         }

         std::array<basic_trigonometric_term, 9> items = {};
   };

   constexpr basic_euler_matrix x_lh = basic_euler_matrix(
      1, 0, 0,
      0, "cos(x)", "sin(x)",
      0, "-sin(x)", "cos(x)"
   );
   constexpr basic_euler_matrix x_rh = basic_euler_matrix(
      1, 0, 0,
      0, "cos(x)", "-sin(x)",
      0, "sin(x)", "cos(x)"
   );


   enum class operation {
      add,
      mul,
   };

   using number_type = double;
   constexpr size_t term_budget_per_expression = 10;

   class constant_term;
   class trigonometric_term;
   class term_group;
   class base_term {
      protected:
         enum class term_type {
            unspecified = -1, // IntelliSense is a dumbass and INSISTS this be initialized outside of the fucking constructor
            constant,
            trigonometric,
            group,
         };
         template<typename T> static constexpr const term_type typeof = []() {
            if constexpr (std::is_same_v<T, constant_term>) {
               return term_type::constant;
            } else if constexpr (std::is_same_v<T, trigonometric_term>) {
               return term_type::trigonometric;
            } else if constexpr (std::is_same_v<T, term_group>) {
               return term_type::group;
            } else {
               cobb::unreachable();
            }
         }();

         const term_type _type = term_type::unspecified;
         
         constexpr base_term(term_type t) : _type(t) {}

      public:
         template<typename T> constexpr bool is() const noexcept {
            return _type == typeof<T>;
         }
         template<typename T> constexpr T* as() noexcept {
            return is<T>() ? static_cast<T*>(this) : nullptr;
         }
         template<typename T> constexpr const T* as() const noexcept {
            return is<T>() ? static_cast<const T*>(this) : nullptr;
         }
   };

   class constant_term : public base_term {
      public:
         constexpr constant_term() : base_term(term_type::constant) {}
         constexpr constant_term(number_type v) : base_term(term_type::constant), value(v) {}
         
         number_type value = 0;

         constexpr bool is_constant() const { return true; }
         constexpr number_type evaluate() const { return value; }
   };

   class trigonometric_term : public base_term {
      public:
         constexpr trigonometric_term() : base_term(term_type::trigonometric) {}

         char angle  = 'x';
         bool negate = false;
         trigonometric_function trig = trigonometric_function::none;

         constexpr bool is_constant() const { return false; }
         constexpr number_type evaluate() const { return NAN; }
   };

   class term_group : public base_term {
      public:
         constexpr term_group() : base_term(term_type::group) {}
         constexpr term_group(operation o) : base_term(term_type::group), type(o) {}
         constexpr term_group(number_type n) : base_term(term_type::group) {
            this->type = operation::add;
            this->terms[this->size++] = new constant_term(n);
         }

         constexpr ~term_group() {
            clear();
         }

         constexpr void clear() {
            for (auto*& term : terms) {
               if (term) {
                  if (auto* casted = term->as<constant_term>()) {
                     delete casted;
                  } else if (auto* casted = term->as<trigonometric_term>()) {
                     delete casted;
                  } else if (auto* casted = term->as<term_group>()) {
                     delete casted;
                  } else {
                     delete term;
                  }
                  term = nullptr;
               }
            }
            this->size = 0;
         }

         operation type = operation::add;
         std::array<base_term*, term_budget_per_expression> terms = {};
         size_t size = 0;

         constexpr void push_back(base_term* t) {
            this->terms[this->size] = t;
            ++this->size;
         }

         constexpr bool is_constant() const {
            for (auto* term : terms) {
               //if (!term->is_constant())
                  return false;
            }
            return true;
         }
         constexpr number_type evaluate() const {
            number_type value = 0;
            if (type == operation::mul)
               value = 1;

            for (size_t i = 0; i < this->size; ++i) {
               auto* term = this->terms[i];

               number_type cv = 0;
               if (const auto* ct = term->as<constant_term>()) {
                  cv = ct->value;
               } else if (const auto* gt = term->as<term_group>()) {
                  cv = gt->evaluate();
               } else {
                  cv = NAN;
               }
               switch (type) {
                  case operation::add: value += cv; break;
                  case operation::mul: value *= cv; break;
               }
            }

            return value;
         }
   };

   class term_matrix {
      public:
         std::array<term_group, 9> terms;
   };


   constexpr auto test = []() {
      term_group group;
      group.type = operation::mul;
      
      group.push_back(new constant_term(3));
      group.push_back(new constant_term(7));

      //return group.evaluate(); // IntelliSense chokes and dies
      auto result = group.evaluate();
      group.clear();
      return result;
   }();
}