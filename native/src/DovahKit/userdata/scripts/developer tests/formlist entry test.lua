local window = ui.window.new()
window:set_layout("grid")
window.title = "FormList tests"

local current_form_list = nil

do
   local picker = ui.formpicker.new()
   local button = ui.button.new("Dump entries")
   window:add_child(picker, 1, 1)
   window:add_child(button, 1, 2)

   picker.form_types = form_types.formlist

   button:on("OnActivated", "", function()
      local form = picker.form
      current_form_list = form
      if form then
         dovah.log_message("[FLST:%s]%s:", form:form_id_to_string(), form.editor_id)
         for i, v in ipairs(form.entries) do
            dovah.log_message(" - %s", v.editor_id)
         end
      end
   end)
end

do
   local picker = ui.formpicker.new()
   local button = ui.button.new("Append")
   window:add_child(picker, 2, 1)
   window:add_child(button, 2, 2)
   
   button:on("OnActivated", "", function()
      if current_form_list then
         local form = picker.form
         if form then
            local entries = current_form_list.entries
            entries[#entries + 1] = form
            dovah.log_message("Entry appended")
         end
      end
   end)
end

do
   local picker = ui.spinbox.new()
   local button = ui.button.new("Remove")
   window:add_child(picker, 3, 1)
   window:add_child(button, 3, 2)
   
   picker.decimals = 0
   picker.minimum  = 1
   picker.step     = 1
   
   button:on("OnActivated", "", function()
      if current_form_list then
         current_form_list.entries:remove(picker.value)
         dovah.log_message("Entry removed")
      end
   end)
end

window:show()