local window = ui.window.new()
window:set_layout("grid")
window.title = "Papyrus property test"

local picker_a = ui.formpicker.new()
local picker_b = ui.formpicker.new()
local script_a = ui.dropdown.new()
local script_b = ui.dropdown.new()
local clone    = ui.button.new("Clone All Properties")
local clear    = ui.button.new("Clear All Properties")
local clear_b  = ui.button.new("Clear All (nil assign)")

window:add_child(ui.text.new("Source:"), 1, 1)
window:add_child(picker_a, 1, 2)
window:add_child(script_a, 2, 2)
window:add_child(ui.text.new("Destination:"), 3, 1)
window:add_child(picker_b, 3, 2)
window:add_child(script_b, 4, 2)
window:add_child(clone, 5, 1, 1, 2)
window:add_child(clear, 6, 1, 1, 2)
window:add_child(clear_b, 7, 1, 1, 2)

picker_a:on("OnChanged", "", function()
   script_a:clear()
   script_a.enabled = false
   local form = picker_a.form
   if not form then
      return
   end
   local papyrus = form.papyrus
   if not papyrus then
      return
   end
   local any = false
   for name, script in pairs(papyrus.scripts) do
      any = true
      script_a:append_item(name)
   end
   script_a.enabled = any
end)
picker_b:on("OnChanged", "", function()
   script_b:clear()
   script_b.enabled = false
   local form = picker_b.form
   if not form then
      return
   end
   local papyrus = form.papyrus
   if not papyrus then
      return
   end
   local any = false
   for name, script in pairs(papyrus.scripts) do
      any = true
      script_b:append_item(name)
   end
   script_b.enabled = any
end)

clone:on("OnActivated", "", function()
   local fa = picker_a.form
   local fb = picker_b.form
   if not (fa and fb) then
      return
   end
   if fa == fb then
      dovah.log_message("these are the same form")
      return
   end
   local pa = fa.papyrus
   local pb = fb.papyrus
   if not (pa and pb) then
      dovah.log_message("failed to get papyrus")
      return
   end
   local sna = script_a.selected_text
   local snb = script_b.selected_text
   local sa = pa.scripts[sna]
   local sb = pb.scripts[snb]
   if not (sa and sb) then
      dovah.log_message("failed to get scripts")
      return
   end
   dovah.log_message("SOURCE properties prior to the operation:")
   for name, prop in pairs(sa.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
   dovah.log_message("DEST properties prior to the operation:")
   for name, prop in pairs(sb.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
   sb.properties = sa.properties
   dovah.log_message("Overwrote DEST with SOURCE. Dumping DEST's property list...")
   for name, prop in pairs(sb.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
end)
clear:on("OnActivated", "", function()
   local fb = picker_b.form
   if not fb then
      return
   end
   local pb = fb.papyrus
   if not pb then
      dovah.log_message("failed to get papyrus")
      return
   end
   local snb = script_b.selected_text
   local sb = pb.scripts[snb]
   if not sb then
      dovah.log_message("failed to get the script")
      return
   end
   dovah.log_message("DEST properties prior to the operation:")
   for name, prop in pairs(sb.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
   sb:remove_all_properties()
   dovah.log_message("Cleared DEST. Dumping DEST's property list...")
   for name, prop in pairs(sb.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
end)
clear:on("OnActivated", "", function()
   local fb = picker_b.form
   if not fb then
      return
   end
   local pb = fb.papyrus
   if not pb then
      dovah.log_message("failed to get papyrus")
      return
   end
   local snb = script_b.selected_text
   local sb = pb.scripts[snb]
   if not sb then
      dovah.log_message("failed to get the script")
      return
   end
   dovah.log_message("DEST properties prior to the operation:")
   for name, prop in pairs(sb.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
   sb.properties = nil
   dovah.log_message("Cleared DEST via nil-assign. Dumping DEST's property list...")
   for name, prop in pairs(sb.properties) do
      dovah.log_message("   %s %s", prop.type, name)
   end
end)

window:show()