#include "./file_offset_of_exception.h"
#include <cassert>

#include "dovah/exceptions/file_parse_errors/base_group_error.h"
#include "dovah/exceptions/file_parse_errors/base_record_error.h"
#include "dovah/exceptions/file_parse_errors/base_subrecord_error.h"
#include "dovah/exceptions/file_parse_errors/child_form_group_follows_wrong_parent.h"
#include "dovah/exceptions/file_parse_errors/child_form_group_specifies_bad_parent_id.h"
#include "dovah/exceptions/file_parse_errors/form_override_has_type_mismatch.h"
#include "dovah/exceptions/file_parse_errors/groups_too_deeply_nested.h"
#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
namespace {
   namespace file_parse_errors {
      using namespace dovah::exceptions::file_parse_errors;
   }
}

namespace dovah::load_order_processes::build_form_stubs_from_file::impl {
   extern size_t file_offset_of_exception(std::exception_ptr e_ptr) {
      size_t offset = 0;
      try {
         std::rethrow_exception(e_ptr);
      } catch (file_parse_errors::base_group_error& e) {
         offset = e.group_offset;
      } catch (file_parse_errors::base_record_error& e) {
         offset = e.record_offset;
      } catch (file_parse_errors::base_subrecord_error& e) {
         offset = e.subrecord_offset;
      } catch (file_parse_errors::child_form_group_follows_wrong_parent& e) {
         offset = e.group.offset;
      } catch (file_parse_errors::child_form_group_specifies_bad_parent_id& e) {
         offset = e.group.offset;
      } catch (file_parse_errors::form_override_has_type_mismatch& e) {
         offset = e.overriding_form.record_offset;
      } catch (file_parse_errors::groups_too_deeply_nested& e) {
         assert(!e.group_offsets.empty());
         offset = e.group_offsets.back();
      } catch (file_parse_errors::unexpected_nested_group& e) {
         offset = e.nested_group.offset;
      } catch (...) {
         // give up
      }
      return offset;
   }
}