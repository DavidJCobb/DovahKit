local window = ui.window.new()
window:set_layout("grid")
window.title = "Papyrus property test"

local picker = ui.formpicker.new()
local s_name = ui.textbox.new()
local p_name = ui.textbox.new()

local create = ui.button.new("Create data")

window:add_child(ui.text.new("Script name:"), 2, 1)
window:add_child(s_name, 2, 2)
window:add_child(ui.text.new("Property name:"), 3, 1)
window:add_child(p_name, 3, 2)
window:add_child(picker, 1, 1, 1, 2)
window:add_child(create, 4, 1, 1, 2)

create:on("OnActivated", "", function()
   local form = picker.form
   if not form then
      return
   end
   local papyrus = form.papyrus
   if not papyrus then
      dovah.log_message("DovahKit can't edit Papyrus data for this form.")
      return
   end
   local list   = papyrus.scripts
   local name   = s_name.text
   local script = list[name]
   if not script then
      script = papyrus.scripts:insert(name)
      if not script then
         error("failed to create the script for some reason")
      end
      dovah.log_message("Created script %s on form.", script.name)
   end
   list = script.properties
   name = p_name.text
   local prop = list[name]
   if not prop then
      prop = script.properties:insert(name)
      if not prop then
         error("failed to create the property for some reason")
      end
      dovah.log_message("Created property %s on script %s on form.", prop.name, script.name)
   else
      dovah.log_message("Property %s already exists on script %s.", prop.name, script.name)
   end
end)

window:show()