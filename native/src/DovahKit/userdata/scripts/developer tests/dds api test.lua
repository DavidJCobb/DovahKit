local window = ui.window.new()
local panel  = ui.widget.new()
local render = ui.image_widget.new()
window:set_layout("ltr")
window:add_child(render)
window:add_child(panel)

render.min_height = 32
render.min_width  = 32

local loaded_dds = nil
local update     = nil -- forward-declared function

do
   panel:set_layout("grid")
   panel.layout_margins = 0
   
   local in_path = ui.textbox.new()
   local in_load = ui.button.new("Load DDS")
   panel:add_child(in_path, 1, 1)
   panel:add_child(in_load, 1, 2)
   in_load:on("OnActivated", "", function()
      local path  = in_path.text
      local asset = dovah.lookup_game_asset(path)
      if dovah.type(asset) ~= "dds_resource" then
         dovah.log_message("Not a DDS: %s", path)
         return
      end
      loaded_dds = asset
      update(true)
   end)
   
   local in_array = ui.spinbox.new()
   in_array.minimum  = 1
   in_array.decimals = 0
   panel:add_child(ui.text.new("Array entry:"), 2, 1)
   panel:add_child(in_array, 2, 2)
   in_array:on("OnChanged", "", function()
      update()
   end)
   
   local in_cube = ui.dropdown.new()
   panel:add_child(ui.text.new("Cubemap face:"), 3, 1)
   panel:add_child(in_cube, 3, 2)
   in_cube:append_item("+X")
   in_cube:append_item("-X")
   in_cube:append_item("+Y")
   in_cube:append_item("-Y")
   in_cube:append_item("+Z")
   in_cube:append_item("-Z")
   in_cube:on("OnChanged", "", function()
      update()
   end)
   local CUBEMAP_INDEX_TO_FACE = { "x_pos", "x_neg", "y_pos", "y_neg", "z_pos", "z_neg" }
   
   local on_mip = ui.checkbox.new("Mipmap:")
   local in_mip = ui.spinbox.new()
   in_mip.decimals = 0
   in_mip.minimum  = 1
   panel:add_child(on_mip, 4, 1)
   panel:add_child(in_mip, 4, 2)
   on_mip:on("OnChanged", "", function(state)
      if state == "checked" then
         in_mip.enabled = true
      else
         in_mip.enabled = false
      end
      update()
   end)
   in_mip:on("OnChanged", "", function()
      update()
   end)
   
   do
      local _lock = false -- lock to protect against widget change events triggered by the update process below
      local function _make_lock_guard()
         _lock = true
         return setmetatable({}, {
            __close = function()
               _lock = false
            end
         })
      end
      update = function(new_image)
         if _lock then
            dovah.log_message("Busy... (%s)", tostring(new_image))
            return
         end
         local guard <close> = _make_lock_guard()
         if new_image then
            in_array.value = 1
            on_mip.checked = false
            in_mip.enabled = false -- because on_mip is unchecked
            in_mip.value   = 1
            --
            if loaded_dds then
               local list  = loaded_dds.images
               local first = list[1]
               in_cube.enabled  = loaded_dds.is_cubemap
               in_array.enabled = #list > 1
               if first then
                  local count = #(first.mipmaps)
                  if count > 0 then
                     on_mip.enabled = true
                     in_mip.maximum = count
                  end
               else
                  on_mip.enabled = false
               end
            else
               in_array.enabled = false
               in_cube.enabled  = false
               on_mip.enabled   = false
               in_mip.enabled   = false
               return
            end
         end
         --
         local content = nil
         if loaded_dds then
            local image = loaded_dds.images[in_array.value]
            if image then
               if loaded_dds.is_cubemap then
                  local face = CUBEMAP_INDEX_TO_FACE[in_cube.selected_index]
                  image = image.cubemap_faces[face]
               end
               if image and on_mip.checked then
                  image = image.mipmaps[in_mip.value]
               end
            end
            if image then
               content = image:copy_to_raster()
            end
         end
         render.image = content
      end
   end
end
update(true)

window:show()
window:set_layout_stretch_at(1, 3)
window:set_layout_stretch_at(2, 2)
panel:set_layout_stretch_at("col", 1, 1)
panel:set_layout_stretch_at("col", 2, 2)