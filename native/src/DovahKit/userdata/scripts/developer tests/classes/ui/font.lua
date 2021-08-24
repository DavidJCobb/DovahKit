label = ui.text.new("The quick brown fox jumped over the lazy dog.")

local window = ui.window.new()
window.title = "Font property test"

window:set_layout("down")
window:add_child(label)
do
   local font  = label.font
   local panel = ui.widget.new()
   window:add_child(panel)
   --
   panel:set_layout("grid")
   panel.layout_margins = 0
   local append_row = nil
   do
      local row = 0
      function append_row()
         row = row + 1
         local combined = ui.widget.new()
         combined:set_layout("ltr")
         combined.layout_margins = 0
         combined:set_layout_stretch_at(1, 1)
         panel:add_child(combined, row, 1)
         return combined
      end
   end
   --
   local row = 0
   do -- Font family and size
      local combined = append_row()
      combined:set_layout_stretch_at(1, 1)
      combined:set_layout_stretch_at(3, 0)
      do -- Font family
         local c = ui.textbox.new(font.family or "")
         c.placeholder = "font family"
         c.text        = font.family or ""
         c:on("OnChanged", "", function(t)
            if t == "" then
               t = nil
            end
            font.family = t
         end)
         combined:add_child(c)
      end
      combined:add_child(ui.line.new("vertical"))
      do
         local size = ui.spinbox.new()
         local unit = ui.dropdown.new()
         combined:add_child(size)
         combined:add_child(unit)
         do
            size.minimum  =  1
            size.value    = 12
            size.maximum  = 99
            size.decimals =  0
            size:on("OnChanged", "", function(v)
               local u = unit.selected_text or "px"
               font.size = math.tointeger(v) .. u
            end)
         end
         do
            unit:append_item("px")
            unit:append_item("pt")
            unit:on("OnChanged", "", function(i)
               local u = unit.selected_text or "px"
               font.size = math.tointeger(size.value) .. u
            end)
         end
         local s = font.size
         if s then
            size.value        = tonumber(tostring(s):gsub("%D+", ""))
            unit.current_text = tostring(s):gsub("%d+", "")
         end
      end
   end
   do -- Capitalization
      local combined = append_row()
      --
      combined:add_child(ui.text.new("Capitalization:"))
      do
         local c = ui.dropdown.new()
         c:append_item("normal")
         c:append_item("uppercase")
         c:append_item("lowercase")
         c:append_item("small caps")
         c:append_item("capitalize")
         c:on("OnChanged", "", function(i)
            font.capitalization = c.items[i].text
         end)
         combined:add_child(c)
      end
   end
   do -- Italics
      local combined = append_row()
      --
      local c = ui.checkbox.new("Italics")
      c.checked = font.italics or false
      c:on("OnToggled", "", function(checked)
         font.italics = checked
      end)
      combined:add_child(c)
   end
   do -- Letter spacing
      local combined = append_row()
      --
      combined:add_child(ui.text.new("Letter spacing:"))
      do
         local c = ui.spinbox.new()
         c.decimals =     2
         c.minimum  = -1000
         c.maximum  =  1000
         c.value    = font.letter_spacing or 0
         c:on("OnChanged", "", function(v)
            font.letter_spacing = v
         end)
         combined:add_child(c)
      end
   end
   do -- Overline
      local combined = append_row()
      --
      local c = ui.checkbox.new("Overline")
      c.checked = font.overline or false
      c:on("OnToggled", "", function(checked)
         font.overline = checked
      end)
      combined:add_child(c)
   end
   do -- Strikethrough
      local combined = append_row()
      --
      local c = ui.checkbox.new("Strikethrough")
      c.checked = font.strikethrough or false
      c:on("OnToggled", "", function(checked)
         font.strikethrough = checked
      end)
      combined:add_child(c)
   end
   do -- Underline
      local combined = append_row()
      --
      local c = ui.checkbox.new("Underline")
      c.checked = font.underline or false
      c:on("OnToggled", "", function(checked)
         font.underline = checked
      end)
      combined:add_child(c)
   end
   do -- Weight
      local combined = append_row()
      --
      combined:add_child(ui.text.new("Weight:"))
      do
         local c = ui.spinbox.new()
         c.decimals =   0
         c.minimum  =   1
         c.maximum  = 100
         c.value    = font.weight or 50
         c:on("OnChanged", "", function(v)
            font.weight = v
         end)
         combined:add_child(c)
      end
   end
   do -- Width
      local combined = append_row()
      --
      local check = ui.checkbox.new("Width:")
      local spin  = ui.spinbox.new()
      combined:add_child(check)
      combined:add_child(spin)
      do
         check:on("OnToggled", "", function(checked)
            if checked then
               font.width = spin.value
            else
               font.width = nil
            end
         end)
      end
      do
         spin.decimals =    0
         spin.minimum  =    1
         spin.maximum  = 4000
         spin.value    = font.width or 100
         spin:on("OnChanged", "", function(v)
            if check.checked then
               font.width = v
            else
               font.width = nil
            end
         end)
      end
   end
   do -- Word spacing
      local combined = append_row()
      --
      combined:add_child(ui.text.new("Word spacing:"))
      do
         local c = ui.spinbox.new()
         c.decimals =     2
         c.minimum  = -1000
         c.maximum  =  1000
         c.value    = font.word_spacing or 0
         c:on("OnChanged", "", function(v)
            font.word_spacing = v
         end)
         combined:add_child(c)
      end
   end
   --
end
window:show()