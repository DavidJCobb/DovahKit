local count = 0
local seen  = 0

function counter(form)
   seen = seen + 1
   if (form.flags & 0x400) ~= 0 then
      count = count + 1
   end
   if seen % 100000 == 0 then
      dovah.log_message("Processing #" .. seen .. "...")
   end
end

dovah.for_each_form_of_type(form_types.actor, counter)
dovah.for_each_form_of_type(form_types.reference, counter)
dovah.for_each_form_of_type(form_types.missile, counter)
dovah.for_each_form_of_type(form_types.arrow, counter)
dovah.for_each_form_of_type(form_types.grenade, counter)
dovah.for_each_form_of_type(form_types.beam, counter)
dovah.for_each_form_of_type(form_types.flame, counter)
dovah.for_each_form_of_type(form_types.cone, counter)
dovah.for_each_form_of_type(form_types.barrier, counter)
dovah.for_each_form_of_type(form_types.placed_hazard, counter)

dovah.log_message("Done!")
dovah.log_message("Persistent refs: %d / %d", count, seen)