#pragma once
#include <atomic>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <QDateTime>
#include "helpers/passkey.h"
#include "dovah/form_types.h"

namespace dovahkit::subsystems::papyrus {
   class core;
   class known_script_ptr;

   class known_script {
      public:
         struct per_file_info {
            std::string docstring;
            struct {
               std::string name;
               known_script* target = nullptr; // NOTE: We do not create `known_script` instances for hardcoded or SKSE types.

               // REMINDER: The value `dovah::form_type::none` represents the base Form class.
               // REMINDER: This enumeration also has values for aliases.
               std::optional<dovah::form_type> underlying_type;
            } extends;
            struct {
               bool conditional = false;
               bool hidden      = false;
            } flags;
         };

         struct loose_file_metadata {
            QDateTime lastmod;
            size_t    size = 0;

            bool operator==(const loose_file_metadata&) const = default;
         };
         struct loose_file_info : public per_file_info {
            loose_file_metadata file_metadata;
         };

         using refcount_passkey  = cobb::passkey<known_script_ptr, known_script>;
         using subsystem_passkey = cobb::passkey<core, known_script>;

      public:
         std::string name;
         struct {
            std::optional<per_file_info>   packed;
            std::optional<loose_file_info> loose;
         } info;
         struct {
            bool cyclical = false;

            known_script* root_class = nullptr;
            struct {
               std::vector<known_script*> loose;
               std::vector<known_script*> packed;
            } potential_subclasses;
         } inheritance;
      protected:
         std::atomic<size_t> refcount = 0; // TODO: use refcounted pointers when mapping form-stubs to known scripts

      public:
         ~known_script();

         constexpr bool exists() const noexcept {
            return info.packed.has_value() || info.loose.has_value();
         }

         constexpr bool name_matches(std::string_view) const;
         constexpr const known_script* superclass() const;
         constexpr known_script* superclass();

         constexpr std::optional<dovah::form_type> underlying_type() const;
         constexpr bool is_attachable_to(dovah::form_type) const;
         constexpr bool is_of_type(std::string_view desired) const; // DOES NOT handle hardcoded scriptnames representing form/alias types.

         constexpr bool is_descendant_of(const known_script&) const;

         bool is_unreferenced() const;
         bool is_unreferenced_except_by_loose() const;

         template<typename Functor> requires (std::is_invocable_v<Functor, const known_script&>)
         constexpr void for_each_child_class(Functor&&) const;
         //
         template<typename Functor> requires (std::is_invocable_v<Functor, known_script&>)
         constexpr void for_each_child_class(Functor&&);

         template<typename Functor> requires (std::is_invocable_v<Functor, const known_script&>)
         constexpr void for_each_descendant_class(Functor&&) const;
         //
         template<typename Functor> requires (std::is_invocable_v<Functor, known_script&>)
         constexpr void for_each_descendant_class(Functor&&);

         #pragma region Passkeyed accessors for the Papyrus subsystem
         void receive_archived_subclass(subsystem_passkey, known_script& subclass);
         //
         void receive_loose_subclass(subsystem_passkey, known_script& subclass);
         void abandon_loose_subclass(subsystem_passkey, known_script& subclass);

         // Non-recursive; used when initially loading all scripts.
         void _compute_root_class(subsystem_passkey);

         // Recursive: used when the class hierarchy has changed (e.g. because a loose file was added/edited/deleted).
         void _update_descendants_root_class(subsystem_passkey);
         #pragma endregion

         #pragma region Passkeyed accessors for refcounting pointers
         void _on_reference_gained(refcount_passkey);
         void _on_reference_lost(refcount_passkey);
         #pragma endregion

         // You can't do something like `auto& a = (info.loose.has_value() ? info.loose : info.packed)` 
         // because the loose and packed infos have different types (one is a subclass of the other). 
         // You can use a static cast to allow the ternary itself to compile, but that isn't wise, and 
         // worse: the compiler may make incredibly egregious mistakes afterward. For example, calls 
         // like `auto& b = (/*ternary here*/).emplace()` *should* cause object slicing, which isn't 
         // great; but what they *actually cause* is MSVC constructing the value ON THE STACK, OUTSIDE 
         // OF EITHER OPTIONAL, and not telling you.
         // 
         // (I'm baffled as to how that particular problem even happened, but it probably has something 
         // to do with reference initialization incorrectly extending the lifetime of [something that 
         // shouldn't have been] a temporary.)
         //
         // Anyway, use these accessors instead.
         //
         #pragma region Accessors to prevent compiler insanity
         constexpr per_file_info* _access_file_info(subsystem_passkey, bool loose) {
            if (loose) {
               if (auto& o = this->info.loose; o.has_value())
                  return &o.value();
               return nullptr;
            }
            if (auto& o = this->info.packed; o.has_value())
               return &o.value();
            return nullptr;
         }
         constexpr per_file_info& _emplace_file_info(subsystem_passkey, bool loose) {
            if (loose)
               return this->info.loose.emplace();
            else
               return this->info.packed.emplace();
         }
         #pragma endregion
   };
}

#include "./known_script.inl"