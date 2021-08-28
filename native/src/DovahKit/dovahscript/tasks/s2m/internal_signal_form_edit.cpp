#include "internal_signal_form_edit.h"
#include "../../core/subsystems/coordinator.h"
#include "../../../editor/core.h"

namespace {
   using coordinator_passkey = dovahscript::core::subsystems::coordinator::passkey_to<dovahscript::tasks::s2m::internal_signal_form_edit>;
}

namespace dovahscript::tasks::s2m {
   internal_signal_form_edit::internal_signal_form_edit() {
   }
   internal_signal_form_edit::internal_signal_form_edit(dovah::form_stub& stub, bool before) : stub(&stub), before(before) {
   }

   void internal_signal_form_edit::_exec_impl() {
      assert(this->stub);
      //
      // We need to emit the form-modification-imminent signal, but to avoid race conditions 
      // within the rest of the editor frontend and possibly even within the backend, we need 
      // to ensure that we perform this operation in lockstep: script execution cannot be 
      // allowed to continue until the signal is emitted and responded to.
      //
      // If we emit the signal on our own thread, then it will trigger a queued connection, 
      // which means that the script thread may be able to modify the form before the signal 
      // is responded to. There are a few systems that will break if this occurs; for example, 
      // the Object Window will not be able to accurately maintain Use Info counts when one 
      // form is modified to no longer use another, because in order to detect that case, it 
      // has to pre-cache the former's outbound connections when form modification is imminent 
      // (but, explicitly, before it has occurred) and then compare that to the outbound 
      // connections that remain when the form modification is complete.
      //
      // If we emit the signal on the main thread, then it will trigger a direct connection, 
      // calling any registered slots and handlers immediately and synchronously. If we wait 
      // on this (e.g. by using our messaging system to effect it), then we, too, will block.
      //
      // Firing messages from within the wrapper internals feels like a disgusting hack and 
      // a total failure of encapsulation. And it is! But if it works, it works.
      //
      // ------------------------------------------------------------------------------------
      // 
      // The coordinator also needs to know when we signal form edits, both before and after. 
      // Why? The "before" signal and the "after" signal are separate tasks. It's theoretically 
      // possible for the user to abort the script after we send the former but before we send 
      // the latter, thereby leaving that form in a "limbo" state where the frontend is waiting 
      // for an incoming modification forever... nevermind whether the script thread actually 
      // got a chance to make that modification.
      //
      auto& coordinator_s = core::subsystems::coordinator::get();
      if (this->before) {
         coordinator_s.expect_modification_of(coordinator_passkey(), *this->stub);
         emit DovahKitCore::get().formModificationImminent(this->stub);
      } else {
         coordinator_s.on_modification_complete(coordinator_passkey(), *this->stub);
         emit DovahKitCore::get().formModified(stub);
      }
   }
}