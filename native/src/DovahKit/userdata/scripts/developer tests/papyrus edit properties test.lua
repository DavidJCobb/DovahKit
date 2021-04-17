local window = ui.window.new()
local fp     = ui.formpicker.new()
local sp     = ui.dropdown.new()
local pp     = ui.dropdown.new()
local npn    = ui.textbox.new()

window:set_layout("grid")
do
   local layout = ui.widget.new()
   layout:set_layout("grid")
   layout:add_child(ui.text.new("Form:"),     1, 1)
   layout:add_child(fp, 1, 2)
   layout:add_child(ui.text.new("Script:"),   2, 1)
   layout:add_child(sp, 2, 2)
   layout:add_child(ui.text.new("Property:"), 3, 1)
   layout:add_child(pp, 3, 2)
   layout:add_child(ui.text.new("New Property:"), 4, 1)
   layout:add_child(npn, 4, 2)
   layout.layout_margins = 0
   window:add_child(layout, 1, 1, 1, 2)
end

do -- until we add separator elements
   local sep = ui.text.new("---")
   sep.alignment = "center center"
   window:add_child(sep, 2, 1, 1, 2)
end

fp:on("OnChanged", "update sp", function(form)
   sp:clear()
   if form then
      local papyrus = form.papyrus
      if papyrus then
         for n, _ in pairs(papyrus.scripts) do
            sp:append_item(n)
         end
         sp.selected_index = 1
      else
         dovah.log_message("no Papyrus data for this form?")
      end
   end
end)
sp:on("OnChanged", "update pp", function()
   pp:clear()
   --
   local form = fp.form
   if form then
      local papyrus = form.papyrus
      if papyrus then
         local script = papyrus.scripts[sp.selected_text]
         if script then
            if script then
               for n, _ in pairs(script.properties) do
                  pp:append_item(n)
               end
            end
         end
      end
   end
end)

function get_focus_property()
   local form = fp.form
   if form then
      local papyrus = form.papyrus
      if papyrus then
         local script = papyrus.scripts[sp.selected_text]
         if script then
            local nn = npn.text
            if not nn or #nn == 0 then
               return script.properties[pp.selected_text]
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
   end
end

local tp = ui.dropdown.new()
do
   tp:append_item("Bool")
   tp:append_item("Float")
   tp:append_item("Int")
   tp:append_item("String")
   tp:append_item("FormOrAlias")
   tp:append_item("Bool[]")
   tp:append_item("Float[]")
   tp:append_item("Int[]")
   tp:append_item("String[]")
   tp:append_item("FormOrAlias[]")
   --
   local button = ui.button.new("Set Type")
   button:on("OnActivated", "", function()
      local prop = get_focus_property()
      if prop then
         prop.type = tp.selected_text
      end
   end)
   --
   window:add_child(tp, 5, 1)
   window:add_child(button, 5, 2)
end

local ip = ui.spinbox.new()
do
   ip.decimals = 0
   ip.minimum  = 1
   --
   window:add_child(ui.text.new("Value #:"), 6, 1)
   window:add_child(ip, 6, 2)
end

do -- Boolean
   local vp = ui.dropdown.new()
   vp:append_item("False")
   vp:append_item("True")
   --
   local button = ui.button.new("Set Bool")
   button:on("OnActivated", "", function()
      local prop = get_focus_property()
      if prop then
         local v = (vp.selected_index == 2)
         if prop.is_array then
            prop.value[ip.value] = v
         else
            prop.value = v
         end
      end
   end)
   --
   window:add_child(vp, 7, 1)
   window:add_child(button, 7, 2)
end
do -- Float
   local vp = ui.spinbox.new()
   vp.minimum = -(vp.maximum - 1)
   --
   local button = ui.button.new("Set Float")
   button:on("OnActivated", "", function()
      local prop = get_focus_property()
      if prop then
         local v = vp.value
         if prop.is_array then
            prop.value[ip.value] = v
         else
            prop.value = v
         end
      end
   end)
   --
   window:add_child(vp, 8, 1)
   window:add_child(button, 8, 2)
end
do -- Int
   local vp = ui.spinbox.new()
   vp.decimals = 0
   vp.minimum  = -(vp.maximum - 1)
   --
   local button = ui.button.new("Set Int")
   button:on("OnActivated", "", function()
      local prop = get_focus_property()
      if prop then
         local v = vp.value
         if prop.is_array then
            prop.value[ip.value] = v
         else
            prop.value = v
         end
      end
   end)
   --
   window:add_child(vp, 9, 1)
   window:add_child(button, 9, 2)
end
do -- String
   local vp = ui.textbox.new()
   --
   local button = ui.button.new("Set String")
   button:on("OnActivated", "", function()
      local prop = get_focus_property()
      if prop then
         local v = vp.text
         if prop.is_array then
            prop.value[ip.value] = v
         else
            prop.value = v
         end
      end
   end)
   --
   window:add_child(vp, 10, 1)
   window:add_child(button, 10, 2)
end
do -- Form
   local vp = ui.formpicker.new()
   --
   local button = ui.button.new("Set Form")
   button:on("OnActivated", "", function()
      local prop = get_focus_property()
      if prop then
         local v = vp.form
         if prop.is_array then
            prop.value[ip.value] = v
         else
            prop.value = v
         end
      end
   end)
   --
   window:add_child(vp, 11, 1)
   window:add_child(button, 11, 2)
end

window:show()