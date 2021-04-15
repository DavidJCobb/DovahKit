local window = ui.window.new()
local picker = ui.formpicker.new()
local button = ui.button.new("Delete")
local buttn2 = ui.button.new("Echo Last Deleted Form")

window:set_layout("grid")
window:add_child(picker, 1, 1)
window:add_child(button, 1, 2)
window:add_child(buttn2, 1, 3)

local last_deleted = nil

button:on("OnActivated", "", function()
   local form = picker.form
   if form then
      last_deleted = form
      form:delete()
   end
end)
buttn2:on("OnActivated", "", function()
   if last_deleted then
      if not object_is_zombie(last_deleted) then
         dovah.log_message("last-deleted form seems recoverable...") -- wrong
         dovah.log_message(last_deleted.editor_id) -- likely to crash, if it runs
      else
         dovah.log_message("Form is not present (wrapper is a zombie)") -- correct behavior
      end
   else
      dovah.log_message("Form is not present (wrapper tests as nil)") -- implies we failed to save the variable
   end
end)

window:show()