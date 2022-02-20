#include "_component_access.h"
#include "_all.h"
#include <array>

namespace dovah::loaded_forms {
   namespace {
      template<typename T> concept _is_model = std::is_same_v<std::remove_reference_t<T>, components::model> || std::is_same_v<std::remove_reference_t<T>, components::model_ts>;

      template<typename T> concept has_model = requires(T& x) {
         { x.model } -> _is_model;
      };
      template<typename T> concept has_papyrus = requires(T& x) {
         { x.script_data } -> std::same_as<components::papyrus_attachment_data&>;
      };

      template<typename T> components::model* get_model(Form* form) {
         if constexpr (has_model<T>) {
            return &((T*)form)->model;
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
         decltype(&get_model<void>)   model   = nullptr;
         decltype(&get_papyrus<void>) papyrus = nullptr;
      };
      _entry::list_t entries = ([]() {
         _entry::list_t out;
         all_loaded_form_types::for_each([&out]<typename Form>() {
            out[Form::form_type] = _entry{
               .model   = &get_model<Form>,
               .papyrus = &get_papyrus<Form>,
            };
         });
         return out;
      })();
   }

   namespace component_access {
      extern components::model* get_model(Form* form) {
         return (entries[form->formType].model)(form);
      }
      extern components::papyrus_attachment_data* get_papyrus_data(Form* form) {
         return (entries[form->formType].papyrus)(form);
      }
   }
}