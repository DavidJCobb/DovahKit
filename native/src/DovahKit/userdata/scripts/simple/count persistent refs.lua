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
window:add_child(progress)

local text_value_esm
local text_value_esp
do
   local grid = ui.widget.new()
   grid:set_layout("grid")
   grid.layout_margins = 0
   window:add_child(grid)

   local label = ui.text.new("Refs flagged as persistent:")
   grid:add_child(label, 1, 1)
   label = ui.text.new("Refs made persistent by ESP files:")
   grid:add_child(label, 2, 1)
   
   text_value_esm = ui.text.new("0")
   text_value_esp = ui.text.new("0")
   text_value_esm.alignment = "right"
   text_value_esp.alignment = "right"
   grid:add_child(text_value_esm, 1, 2)
   grid:add_child(text_value_esp, 2, 2)

   grid:set_layout_stretch_at("col", 1, 1) -- have to do this after the columns exist
   grid:set_layout_stretch_at("col", 2, 0)
end

window:show()

--

local total     = 0
local count_esm = 0
local count_esp = 0
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
      total     = total + 1
      count_esm = count_esm + 1
   else
      local file = form:get_last_source_file()
      if file then
         if not file.is_master then
            total     = total + 1
            count_esp = count_esp + 1
         end
      else
         --
         -- Refs in hardcoded files should always be persistent, as there's only 
         -- one: the player.
         --
         total     = total + 1
         count_esm = count_esm + 1
      end
   end
   if seen % 100 == 0 then
      progress.value = seen
      text_value_esm.text = count_esm
      text_value_esp.text = count_esp
   end
end

for _, v in pairs(REF_TYPES) do
   dovah.for_each_form_of_type(v, counter)
end
progress.value = progress.maximum
text_value_esm.text = count_esm
text_value_esp.text = count_esp

dovah.log_message("Done!")
dovah.log_message("Persistent refs: %d / %d", total, seen)