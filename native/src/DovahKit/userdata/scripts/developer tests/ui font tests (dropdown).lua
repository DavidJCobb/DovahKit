
local window = ui.window.new()
local picker = ui.dropdown.new()
local append = ui.button.new("Append")
local remove = ui.button.new("Remove Sel'd")
local clrfam = ui.button.new("Clear Families")
local clrall = ui.button.new("Clear All")
local setcal = ui.button.new("Calibri All")

local names = {
   "Arial",
   "Helvetica",
   "Impact",
   "Segoe UI",
   "Times New Roman",
   "Times New Roman Condensed",
   "Times New Roman Condensed 8pt",
   "Times New Roman Condensed 15pt",
}
local fonts = {
   false,
   false,
   { bold = true },
   false,
   false,
   { family = "Times New Roman", width = 80 },
   { family = "Times New Roman", width = 80, size = "8pt" },
   { family = "Times New Roman", width = 80, size = "15pt" },
}

do
   local INITIAL_COUNT = 5
   for i = 1, INITIAL_COUNT do
      local name = names[i]
      local font = fonts[i]
      if not font then
         font = { family = name }
      elseif not font.family then
         font.family = name
      end
      picker:append_item({
         text = name,
         font = font,
      })
   end
end

window:set_layout("grid")
window:add_child(picker, 1, 1, 1, 2)
window:add_child(append, 3, 1)
window:add_child(remove, 3, 2)
window:add_child(clrfam, 4, 1)
window:add_child(clrall, 4, 2)
window:add_child(setcal, 5, 1)

local offset = nil -- earliest removed item in the list
append:on("OnActivated", "", function()
   local items = picker.items
   local count = #items
   if count >= #names then
      return
   end
   local i    = count + 1
   local name = names[i]
   local font = fonts[i]
   if not font then
      font = { family = name }
   elseif not font.family then
      font.family = name
   end
   picker:append_item({
      text = name,
      font = font,
   })
end)
remove:on("OnActivated", "", function()
   local i = picker.selected_index
   if not i then
      return
   end
   if (not offset) or (i < offset) then
      offset = i
   end
   picker:remove_item(i)
end)
clrfam:on("OnActivated", "", function()
   local items = picker.items
   local count = #items
   for i = 1, count do
      local item = items[i]
      local font = item.font
      if font then
         font.family = nil
      end
   end
end)
clrall:on("OnActivated", "", function()
   local items = picker.items
   local count = #items
   for i = 1, count do
      local item = items[i]
      item.font = nil
   end
end)
setcal:on("OnActivated", "", function()
   local items = picker.items
   local count = #items
   for i = 1, count do
      local item = items[i]
      local font = item.font
      if font then
         font.family = "Calibri"
      else
         item.font = { family = "Calibri" }
      end
   end
end)

do
   local NEXT_ROW = 6
   --
   local fields = {
      "bold",
      "italics",
      "letter_spacing",
      "weight",
      "width",
      "word_spacing",
   }
   local values = {
      true,
      true,
      1.5,
      80,
      75,
      4,
   }
   --
   local radio_val = ui.radio_button.new("Test Value")
   local radio_nil = ui.radio_button.new("Nil")
   window:add_child(radio_val, NEXT_ROW,     1, 1, 2)
   window:add_child(radio_nil, NEXT_ROW + 1, 1, 1, 2)
   radio_val.checked = true
   NEXT_ROW = NEXT_ROW + 2
   --
   for i = 1, #fields do
      local button = ui.button.new("Set: " .. fields[i])
      local field  = fields[i]
      local value  = values[i]
      button:on("OnActivated", "", function()
         local v = nil
         if radio_val.checked then
dovah.log_message("setting")
            v = value
         else
dovah.log_message("clearing")
         end
         --
         local items = picker.items
         local count = #items
         for i = 1, count do
            local item = items[i]
            local font = item.font
            if font then
               font[field] = v
            else
               local t = {}
               t[field] = v
               item.font = t
            end
         end
      end)
      --
      local r = math.floor(NEXT_ROW + (i - 1) / 2)
      local c = math.floor((i - 1) % 2) + 1
      window:add_child(button, r, c)
   end
end

window:show()