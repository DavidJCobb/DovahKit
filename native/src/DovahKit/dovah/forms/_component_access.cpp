#include "_component_access.h"
#include "_all.h"
#include <array>

namespace dovah::loaded_forms {
   namespace {
      template<typename T> concept _is_model = std::is_same_v<std::remove_reference_t<T>, components::model> || std::is_same_v<std::remove_reference_t<T>, components::model_ts>;

      template<typename T> concept has_leveled_list = requires(T & x) {
         { x.leveled_list_data } -> std::common_with<components::leveled_list&>;
      };
      template<typename T> concept has_model = requires(T& x) {
         { x.model } -> _is_model;
      };
      template<typename T> concept has_object_bounds = requires(T & x) {
         { x.bounds } -> std::same_as<components::object_bounds&>;
      };
      template<typename T> concept has_papyrus = requires(T& x) {
         { x.script_data } -> std::same_as<components::papyrus_attachment_data&>;
      };

      template<typename T> components::leveled_list* get_leveled_list(Form* form) {
         if constexpr (has_leveled_list<T>) {
            return &((T*)form)->leveled_list_data;
         }
         return nullptr;
      }
      template<typename T> components::model* get_model(Form* form) {
         if constexpr (has_model<T>) {
            return &((T*)form)->model;
         }
         return nullptr;
      }
      template<typename T> components::object_bounds* get_object_bounds(Form* form) {
         if constexpr (has_object_bounds<T>) {
            return &((T*)form)->bounds;
         }
         return nullptr;
      }
      template<typename T> components::papyrus_attachment_data* get_papyrus(Form* form) {
         if constexpr (has_papyrus<T>) {
            return &((T*)form)->script_data;
         }
         return nullptr;
      }

      struct _entry {
         using list_t = std::array<_entry, form_types.size()>;
         //
         decltype(&get_leveled_list<void>)  leveled_list  = nullptr;
         decltype(&get_model<void>)         model         = nullptr;
         decltype(&get_object_bounds<void>) object_bounds = nullptr;
         decltype(&get_papyrus<void>)       papyrus       = nullptr;
      };
      _entry::list_t entries = ([]() {
         _entry::list_t out;
         all_loaded_form_types::for_each([&out]<typename Form>() {
            out[(size_t)Form::form_type] = _entry{
               .leveled_list  = &get_leveled_list<Form>,
               .model         = &get_model<Form>,
               .object_bounds = &get_object_bounds<Form>,
               .papyrus       = &get_papyrus<Form>,
            };
         });
         return out;
      })();
   }

   namespace component_access {
      extern components::leveled_list* get_leveled_list(Form* form) {
         return (entries[(size_t)form->type].leveled_list)(form);
      }
      extern components::model* get_model(Form* form) {
         return (entries[(size_t)form->type].model)(form);
      }
      extern components::object_bounds* get_object_bounds(Form* form) {
         return (entries[(size_t)form->type].object_bounds)(form);
      }
      extern components::papyrus_attachment_data* get_papyrus_data(Form* form) {
         return (entries[(size_t)form->type].papyrus)(form);
      }
   }
}