
/*
   Assumes that macro names `X` and `FORM_DATA_PARAMS` are not defined and 
   are free to use as scratch macros. (They'll be undef'd after we're done 
   with them.)

   Given:

      Params
         If this class is, or is nested in, a class template whose parameter 
         is a form-data-params type, that parameter's name must be Params, 
         or you must define a macro that substitutes in the parameter's name.

      CLASSNAME
         Macro. The identifier of the current class -- not fully-qualified, 
         and without template parameters.

      TYPENAME
         Macro. The name of the current class, qualified out to and including 
         the containing template that supplies form-data-params. That template 
         should be given parameter `FORM_DATA_PARAMS`.

         Examples:
            CLASSNAME<FORM_DATA_PARAMS>
            some_containing_template<FORM_DATA_PARAMS>::foo::CLASSNAME

      PER_FIELD
         Macro. An X-macro that invokes its parameter for each field in the 
         current class.

   The CLASSNAME, TYPENAME, and PER_FIELD macros will be undef'd for you at 
   the end of this file, after we finish defining the class members we need. 
   Those members are:

      form_data_params_type

         A type alias that resolves to `Params`. Used so that the params are 
         retrievable for nested classes defined inside of a form-data class; 
         our machinery for form-data-op functions requires this.
      
      visit_members(T, visitor)

         A static member function which takes a reference to a class instance, 
         and a callable, and invokes the callable for every field in the class 
         instance. The callable can be a capturing or non-capturing lambda. 
         The value can be an l-value or r-value reference, and can be const or 
         non-const.

         Useful for implementing things like a "clear managed data" function 
         or a "sever outbound references to this form" function.
      
      visit_members_in_tandem(T, U, visitor)

         A static member function which takes two class instances, potentially 
         with different form-data-params, and invokes the visitor on each pair 
         of matching fields within the two instances. The visitor can be a 
         capturing or non-capturing lambda. The arguments can be l-value or 
         r-value references, and can be const or non-const.

         Useful for implementing comparison operators, and assignment operators 
         that can convert between (un)managed form data.
*/

using form_data_params_type = Params;

#pragma region T::visit_fields(T&, visitor)
   #define X(fieldname, ...) std::forward<Visitor>(visitor)(v.fieldname);
   
   template<typename Visitor>
   static constexpr void visit_fields(CLASSNAME& v, Visitor&& visitor) {
      PER_FIELD(X)
   }
   template<typename Visitor>
   static constexpr void visit_fields(const CLASSNAME& v, Visitor&& visitor) {
      PER_FIELD(X)
   }

   #undef X
#pragma endregion

#pragma region T::visit_fields_in_tandem(T&, Lambda)
   #define X(fieldname, ...) std::forward<Visitor>(visitor)(a.fieldname, b.fieldname);
   
   template<typename Visitor, typename A, typename B>
      requires requires {
         //
         // Given T = the templated type that this function is a member of, 
         // this constraint requires that the first two arguments to this 
         // function be instances of T given *some* set of form-data-params; 
         // the two instances need not be templated on th *same* params. The 
         // arguments can be any reference (r/l) and constness, and may 
         // differ from each other in either respect.
         //
         typename std::decay_t<A>::form_data_params_type;
         typename std::decay_t<B>::form_data_params_type;
         requires ([]() consteval -> bool {
            using Decayed = std::decay_t<A>;
            #define FORM_DATA_PARAMS typename Decayed::form_data_params_type
            using Desired = TYPENAME;
            #undef FORM_DATA_PARAMS
            return std::is_same_v<Decayed, Desired>;
         }());
         requires ([]() consteval -> bool {
            using Decayed = std::decay_t<B>;
            #define FORM_DATA_PARAMS typename Decayed::form_data_params_type
            using Desired = TYPENAME;
            #undef FORM_DATA_PARAMS
            return std::is_same_v<Decayed, Desired>;
         }());
         /*
            I realize that this requires some explanation.

            We want this function to be able to handle any combination of 
            type modifiers on A and B: lvalue versus rvalue; const versus 
            non-const. The naive approach would be to define an overload 
            for each combination, giving us 16 possible overloads. The 
            smarter approach is to use "universal references," also known  
            as "forwarding references."

            Forwarding references use the same syntax as rvalue references 
            and exploit a quirk in "reference collapsing." If you have a 
            template type parameter that is *fully* type-deduced, and the 
            parameter is written as `T&&`, then an r-value deduces to the 
            type `X&& &&`, which collapses to `X&&`, while an l-value will 
            deduce to `X& &&` and collapse to `X&`. The problem, of course, 
            is that if the type is not 100% deduced, then this behavior 
            will not happen: `const T&&` and `T<U>&&` do not have their 
            "referenceness" deduced and so are always r-value references.

            Thus, we need our parameters to just be bare `A&&` and `B&&` 
            in order for them to be universal references, so we can avoid 
            having to write sixteen overloads. We can't e.g. take the 
            form-data-params types as template parameters and then have 
            the argument types be of the form `TYPENAME<Params>&&` (given 
            that we define `FORM_DATA_PARAMS` as the appropriate template 
            parameter's name).

            However, using bare `A&&` and `B&&` costs us the type-checking 
            that we'd get from writing `TYPENAME<Params>`.

            The obvious solution is to move type-checks into a constraint. 
            However, that runs into another issue. Suppose that we were to 
            take the lambdas above, and inline the checks they perform 
            directly into this `requires` constraint. Of course, you can't 
            have a type alias (`using`) in a constraint, so we'd have to 
            spell the types out "long-ways" in the `std::is_same_v` check... 
            and then things break. The problem is that `TYPENAME` can take 
            either of the following forms --

               some_type<FORM_DATA_PARAMS>
               some_type<FORM_DATA_PARAMS>::some_nested_type
            
            -- and the latter is ambiguous in most contexts, including when 
            passed as a template parameter -- even to a template that only 
            permits a type there. Adding `typename` fixes the second case, 
            but breaks the first, because `typename` is a syntax error if 
            it's followed by a type that isn't a `nested-name-specifier`.

            However, when you define a type alias (`using`), the content on 
            the righthand side is always interpreted as a type name; it's 
            not considered ambiguous in that context.

            Thus, the IIFEs. We can define type aliases inside of those, 
            feed those type aliases into `std::is_same_v`, and have the 
            IIFEs return the value that that template produces.

            It's cursed, but it works.
         */
      }
   static constexpr void visit_fields_in_tandem(
      A&& a,
      B&& b,
      Visitor&& visitor
   ) {
      //
      #define FORM_DATA_PARAMS typename std::decay_t<A>::form_data_params_type
      using expected_a_type = TYPENAME;
      #undef FORM_DATA_PARAMS
      static_assert(std::is_same_v<expected_a_type, std::decay_t<A>>);
      //
      PER_FIELD(X)
   }
   
   #undef X
#pragma endregion

#undef PER_FIELD
#undef TYPENAME
#undef CLASSNAME