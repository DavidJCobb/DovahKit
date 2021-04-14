local window = ui.window.new()
local picker = ui.dropdown.new()
local button = ui.button.new("Toggle Sort")
local append = ui.button.new("Append")
local remove = ui.button.new("Remove Sel'd")
local remove2 = ui.button.new("Remove by UD")

local readout = ui.text.new()

local names = {
   "Ari",
   "Chris",
   "Brianna",
   "Quigley",
   "Lucrezia",
   "Farah",
   "Genevieve",
   "Wilhelmina",
   "Anton",
   "Logan",
   "Griselda",
   "June",
   "Klaus",
   "Liam",
   "Morgan",
   "Nathan",
   "Elise",
   "Cherie",
   "Aurelie",
   "Vladimir",
   "Misha",
   "Holden",
   "Hannibal",
   "Ryan",
   "Ash",
   "Alex",
   "Sarah",
   "Piper",
   "Marie",
   "Johnny",
   "Misty",
   "Saul",
   "Walter",
   "Jesse",
   "Mike",
   "Skyler",
}

for i = 1, 5 do
   picker:append_item(names[i] .. " (" .. i .. ")")
end

window:set_layout("grid")
window:add_child(picker, 1, 1, 1, 2)
window:add_child(button, 1, 3)
window:add_child(readout, 2, 1, 1, 3)
window:add_child(append, 3, 1)
window:add_child(remove, 3, 2)
window:add_child(remove2, 3, 3)

button:on("OnActivated", "", function()
   picker.sorted = not picker.sorted
end)

local offset = nil -- earliest removed item in the list
append:on("OnActivated", "", function()
   local count = #picker.items
   if count >= #names then
      return
   end
   local i    = count + 1
   local name = names[i] .. " (" .. i .. ")"
   picker:append_item(name)
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
remove2:on("OnActivated", "", function()
   local item = picker.selected_item
   if not item then
      return
   end
   picker:remove_item(item)
   if item and not object_is_zombie(item) then
      error("failed to zombify the wrapper?")
   end
end)


--
-- Unit test: Lua should always work with logical indices, not visible 
-- indices. This means that even if you enable sorting, such that Quigley 
-- becomes the fifth item visible in the dropdown, picking him should have 
-- Lua detect him as selected index 4 of 5. Event listeners that receive 
-- combobox indices should behave similarly.
--
picker:on("OnChanged", "readout", function(index)
   readout.text = "E: " .. tostring(index) .. " / W: " .. tostring(picker.selected_index)
end)

window:show()