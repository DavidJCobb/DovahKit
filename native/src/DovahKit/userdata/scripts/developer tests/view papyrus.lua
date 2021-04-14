local window = ui.window.new()
window:set_layout("grid")
window.title = "Papyrus viewer"

local fp = ui.formpicker.new()
local sp = ui.dropdown.new()
local pp = ui.dropdown.new()
local ap = ui.spinbox.new() -- array item picker
local al = ui.text.new()

sp.sorted = true
pp.sorted = true
ap.minimum = 1
ap.decimals = 0

window:add_child(fp, 1, 1, 1, 2)
window:add_child(sp, 2, 1, 1, 2)
window:add_child(pp, 3, 1, 1, 2)
window:add_child(ap, 4, 1)
window:add_child(al, 4, 2)
window:set_layout_stretch_at("col", 1, 0)
window:set_layout_stretch_at("col", 2, 1)

function get_current_papyrus_root()
   local form = fp.form
   if not form then
      return
   end
   return form.papyrus
end

function rebuild_script_picker()
   local prior = sp.selected_text
   sp.enabled = false
   sp:clear()
   local papyrus = get_current_papyrus_root()
   if not papyrus then
      return
   end
   local match = nil
   local count = 0
   for n, _ in pairs(papyrus.scripts) do
      sp:append_item(n)
      count = count + 1
      if n == prior then
         match = count
      end
   end
   if match then
      sp.selected_index = match
   end
   sp.enabled = count > 0
end
function rebuild_property_picker()
   local prior = pp.selected_text
   pp.enabled = false
   pp:clear()
   local papyrus = get_current_papyrus_root()
   if not papyrus then
      return
   end
   local script = papyrus.scripts[sp.selected_text]
   if not script then
      return
   end
   local match = nil
   local count = 0
   for n, _ in pairs(script.properties) do
      pp:append_item(n)
      count = count + 1
      if n == prior then
         match = count
      end
   end
   if match then
      pp.selected_index = match
   end
   pp.enabled = count > 0
end
function rebuild_value()
   local prop = nil
   do
      local papyrus = get_current_papyrus_root()
      if papyrus then
         local script = papyrus.scripts[sp.selected_text]
         if script then
            prop = script.properties[pp.selected_text]
         end
      end
   end
   ap.enabled = false
   if not prop then
      ap.value = 1
      al.text  = ""
      return
   end
   local value = prop.value
   if type(value) == "userdata" and not object_is_form(value) then
      ap.enabled = true
      ap.maximum = #value
      local item = value[ap.value]
      if not item then
         ap.value = 1
         item = value[1]
      end
      if item then
         al.text = tostring(item)
      end
   else
      ap.value = 1
      if object_is_form(value) then
         al.text = "[FORM:" .. value:form_id_to_string() .. "]" .. value.editor_id
      else
         al.text = tostring(value)
      end
   end
end

fp:on("OnChanged", "", function(form)
   rebuild_script_picker()
   rebuild_property_picker()
   rebuild_value()
end)
sp:on("OnChanged", "", function(index)
   rebuild_property_picker()
   rebuild_value()
end)
pp:on("OnChanged", "", function(index)
   rebuild_value()
end)
ap:on("OnChanged", "", function(index)
   rebuild_value()
end)

window:show()