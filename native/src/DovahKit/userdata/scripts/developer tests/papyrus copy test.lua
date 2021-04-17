local window = ui.window.new()

local PropertyPicker = {}
PropertyPicker.__index = PropertyPicker
function PropertyPicker:new()
   local out = setmetatable({}, self)
   out.widget = ui.widget.new()
   out.fp     = ui.formpicker.new()
   out.sp     = ui.dropdown.new()
   out.pp     = ui.dropdown.new()
   out.npn    = ui.textbox.new()
   --
   do
      local fp = out.fp
      local sp = out.sp
      local pp = out.pp
      fp:on("OnChanged", "update sp", function(form)
         sp:clear()
         local papyrus = out:get_root(form)
         if papyrus then
            for n, _ in pairs(papyrus.scripts) do
               sp:append_item(n)
            end
            sp.selected_index = 1
         end
      end)
      sp:on("OnChanged", "update pp", function()
         pp:clear()
         --
         local script = out:get_script()
         if script then
            for n, _ in pairs(script.properties) do
               pp:append_item(n)
            end
         end
      end)
   end
   --
   do
      local layout = out.widget
      layout:set_layout("grid")
      layout:add_child(ui.text.new("Form:"),     1, 1)
      layout:add_child(out.fp, 1, 2)
      layout:add_child(ui.text.new("Script:"),   2, 1)
      layout:add_child(out.sp, 2, 2)
      layout:add_child(ui.text.new("Property:"), 3, 1)
      layout:add_child(out.pp, 3, 2)
      layout:add_child(ui.text.new("New Property:"), 4, 1)
      layout:add_child(out.npn, 4, 2)
      layout.layout_margins = 0
   end
   --
   return out
end
function PropertyPicker:get_desired_script_name()
   return self.sp.selected_text
end
function PropertyPicker:get_desired_property_name()
   local nn = self.npn.text
   if not nn or #nn == 0 then
      return self.pp.selected_text
   end
   return nn
end
function PropertyPicker:get_root(form)
   if not form then
      form = self.fp.form
      if not form then
         return
      end
   end
   return form.papyrus
end
function PropertyPicker:get_script(root)
   if not root then
      root = self:get_root()
      if not root then
         return
      end
   end
   return root.scripts[self:get_desired_script_name()]
end
function PropertyPicker:get_property()
   local script = self:get_script()
   if script then
      local nn = self.npn.text
      if not nn or #nn == 0 then
         return script.properties[self.pp.selected_text]
      else
         local prop = script.properties[nn]
         if prop then
            return prop
         end
         script:add_property(nn)
         return script.properties[nn]
      end
   end
end


local src = PropertyPicker:new()
local dst = PropertyPicker:new()

window:set_layout("grid")
window:add_child(ui.text.new("Source"), 1, 1)
window:add_child(ui.line.new("vertical"), 1, 2, 5, 1)
window:add_child(ui.text.new("Destination"), 1, 3)
window:add_child(src.widget, 2, 1)
window:add_child(dst.widget, 2, 3)

do
   local button = ui.button.new("Dump Dst Scripts")
   button:on("OnActivated", "", function()
      local r = dst:get_root()
      dovah.log_message("Dumping scripts for destination...")
      if r then
         for k, v in pairs(r.scripts) do
            dovah.log_message(" - %s", k)
            for n, p in pairs(v.properties) do
               local value = p.value
               local text  = ""
               if p.is_array then
                  text = "{ "
                  local count = #value
                  for i = 1, count do
                     if i > 1 then
                        text = text .. ","
                     end
                     text = text .. tostring(value[i])
                  end
                  text = text .. " }"
               else
                  text = tostring(value)
               end
               dovah.log_message("    - %s %s == %s", p.type, n, text)
            end
         end
      end
      dovah.log_message("Done.")
   end)
   window:add_child(button, 3, 1)
end

do
   local button = ui.button.new("Copy All Scripts")
   button:on("OnActivated", "", function()
      local r1 = dst:get_root()
      local r2 = src:get_root()
      r1.scripts = r2.scripts
   end)
   window:add_child(button, 3, 3)
end
do
   local button = ui.button.new("Copy One Script")
   button:on("OnActivated", "", function()
      local r1 = dst:get_root()
      local r2 = src:get_root()
      local s2 = src:get_script(r2)
      r1.scripts[dst:get_desired_script_name()] = s2
   end)
   window:add_child(button, 4, 3)
end
do
   local button = ui.button.new("Copy All Prop.s")
   button:on("OnActivated", "", function()
      local s1 = dst:get_script()
      local s2 = src:get_script()
      s1.properties = s2.properties
   end)
   window:add_child(button, 5, 3)
end
do
   local button = ui.button.new("Copy One Prop.")
   button:on("OnActivated", "", function()
      local s1 = dst:get_script()
      local p2 = src:get_property()
      s1.properties[dst:get_desired_property_name()] = p2
   end)
   window:add_child(button, 5, 3)
end

window:show()
