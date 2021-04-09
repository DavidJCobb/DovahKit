local window = ui.window.new()
window:set_layout("grid")
window.title = "Papyrus viewer"

local fp = ui.formpicker.new()
local sp = ui.spinbox.new() -- script picker   (we don't have comboboxes yet)
local sl = ui.text.new()
local pp = ui.spinbox.new() -- property picker (we don't have comboboxes yet)
local pl = ui.text.new()
local ap = ui.spinbox.new() -- array item picker
local al = ui.text.new()

sp.minimum = 1
pp.minimum = 1
ap.minimum = 1
sp.decimals = 0
pp.decimals = 0
ap.decimals = 0

window:add_child(fp, 1, 1, 1, 2)
window:add_child(sp, 2, 1)
window:add_child(sl, 2, 2)
window:add_child(pp, 3, 1)
window:add_child(pl, 3, 2)
window:add_child(ap, 4, 1)
window:add_child(al, 4, 2)
window:set_layout_stretch_at("col", 1, 0)
window:set_layout_stretch_at("col", 2, 1)

function update_readouts()
   sl.text = ""
   pl.text = ""
   al.text = ""
   sp.enabled = false
   pp.enabled = false
   ap.enabled = false
   --
   local form = fp.form
   if not form then
      sp.value = 1
      pp.value = 1
      ap.value = 1
      return
   end
   local papyrus = form.papyrus
   if not papyrus then
      sp.value = 1
      pp.value = 1
      ap.value = 1
      return
   end
   local list = papyrus.scripts
   local max  = #list
   sp.maximum = max
   if max <= 0 then
      sp.value = 1
      pp.value = 1
      ap.value = 1
      return
   end
   local script = list[sp.value]
   if not script then
      sp.value = 1
      script = list[1]
      if not script then
         pp.value = 1
         ap.value = 1
         return
      end
   end
   sl.text = script.name
   list    = script.properties
   max     = #list
   pp.maximum = max
   if #list <= 0 then
      pp.value = 1
      ap.value = 1
      return
   end
   pp.enabled = true
   local prop = script.properties[pp.value]
   if not prop then
      pp.value = 1
      prop = list[1]
      if not prop then
         ap.value = 1
         return
      end
   end
   pl.text = prop.name
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
   update_readouts()
end)
sp:on("OnChanged", "", function(index)
   update_readouts()
end)
pp:on("OnChanged", "", function(index)
   update_readouts()
end)
ap:on("OnChanged", "", function(index)
   update_readouts()
end)

window:show()