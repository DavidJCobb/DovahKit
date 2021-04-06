local window = ui.window.new()
local picker = ui.formpicker.new()
local button = ui.button.new("Dump aliases")

picker.form_types = form_types.quest

window:set_layout("down")
window:add_child(picker)
window:add_child(button)

button:on("OnActivated", "click", function()
   local form = picker.form
   dovah.log_message("[QUST:" .. form:form_id_to_string() .. "]")
   for id, alias in pairs(form.aliases_by_id) do
      dovah.log_message("Alias #%s: %s (%s)", tostring(id), alias.name, alias.type)
   end
end)

window:show()