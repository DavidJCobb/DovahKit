local REF_TYPES = {
   form_types.actor,
   form_types.reference,
   form_types.missile,
   form_types.arrow,
   form_types.grenade,
   form_types.beam,
   form_types.flame,
   form_types.cone,
   form_types.barrier,
   form_types.placed_hazard
}

--

local window = ui.window.new()
window.title = "Count persistent refs"
window:set_layout("v")

local caption = ui.text.new("Counting persistent references. Please wait...")
caption.alignment = "center center"
window:add_child(caption)

local progress = ui.progress_bar.new()
progress.alignment = "center center"
progress.format    = "%p% (%v/%m)"
progress.minimum   = 0
progress.maximum   = 0
window:add_child(progress, 2, 0)

window:show()

--

local count = 0
local seen  = 0

do
   local max = 0
   for _, v in pairs(REF_TYPES) do
      max = max + dovah.count_forms_of_type(v)
   end
   progress.maximum = max
end

function counter(form)
   seen = seen + 1
   if (form.flags & 0x400) ~= 0 then
      count = count + 1
   end
   if seen % 100 == 0 then
      progress.value = seen
   end
end

for _, v in pairs(REF_TYPES) do
   dovah.for_each_form_of_type(v, counter)
end
progress.value = progress.maximum

dovah.log_message("Done!")
dovah.log_message("Persistent refs: %d / %d", count, seen)