
local window = ui.window.new()
local panel  = ui.widget.new()
local scroll = ui.scrollbox.new()
local canvas = ui.canvas.new()

local cell_picker = ui.dropdown.new()
local navm_picker = ui.dropdown.new()
local go_button   = ui.button.new()

local progress = ui.progress_bar.new()
progress.alignment = "center center"
function update_progress(text, min, max, value)
   if text then
      progress.format = text
   end
   if min then
      progress.minimum = min
   end
   if max then
      progress.maximum = max
   end
   if value then
      progress.value = value
   end
end
function clear_progress()
   progress:reset()
end

cell_picker.sorted = true

-- Sadly, Dovahscript doesn't expose Qt's ability to associate arbitrary 
-- data with combobox items via `Qt::UserRole + n`. This means we have to 
-- find a form, given a combobox item, through other means. For navmeshes 
-- the combobox item's text is the form ID; for cells, the text is the 
-- editor ID.
--
function get_selected_cell()
   local index = cell_picker.selected_index
   if not index then
      return nil
   end
   local item = cell_picker.items[index]
   if not item then
      return nil
   end
   local editor_id = item.text
   local cell      = nil
   dovah.for_each_form_of_type(
      form_types.cell,
      function(form)
         if form.parent_world then
            return
         end
         if form.editor_id == editor_id then
            cell = form
            return true
         end
      end
   )
   return cell
end
function get_selected_navmesh()
   local index = navm_picker.selected_index
   if not index then
      return nil
   end
   local item = navm_picker.items[index]
   if not item then
      return nil
   end
   local form_id = tonumber(item.text, 16)
   local form    = dovah.get_form_by_id(form_id)
   if form and form.form_type ~= form_types.navmesh then
      return nil
   end
   return form
end

-- Sadly, Dovahscript doesn't (and presently can't) expose custom filters 
-- for DKFormPicker to Lua. Custom filters have to run on the main thread, 
-- and thus can't invoke Lua-thread functions. We also don't offer any 
-- pre-made filters (e.g. "interior cells," or "cells in worldspace," or 
-- "children of cell X with form type Y", or "all forms except these").
--
function rebuild_cell_picker()
   cell_picker:clear()
   update_progress("Finding interior cells...", 0, 0, 0)
   
   panel.enabled = false
   ui.run_when_locked(function()
      dovah.for_each_form_of_type(
         form_types.cell,
         function(cell)
            local editor_id = cell.editor_id
            if (not cell.parent_world) and editor_id ~= "" then
               cell_picker:append_item(cell.editor_id)
            end
         end
      )
      panel.enabled = true
      clear_progress()
   end)
end
function rebuild_navmesh_picker(cell)
   navm_picker.enabled = false
   navm_picker:clear()
   go_button.enabled = false
   if not cell then
      return
   end
   update_progress("Finding navmeshes in cell...", 0, 0, 0)
   ui.run_when_unlocked(function()
      local children = cell:get_all_children()
      local count    = 0
      for i = 1, #children do
         local form = children[i]
         if form.form_type == form_types.navmesh then
            navm_picker:append_item(form:form_id_to_string())
            count = count + 1
         end
      end
      navm_picker.enabled = count > 0
      go_button.enabled   = count > 0
      clear_progress()
   end)
end

do
   window:set_layout("grid")
   window:add_child(scroll, 1, 1)
   window:add_child(panel,  1, 2)
   local sb = scroll.body
   sb:set_layout("grid")
   sb:add_child(canvas)
   
   do
      panel:set_layout("down")
      panel.layout_margins = 0
      do
         local row   = ui.widget.new()
         row:set_layout("ltr")
         row.layout_margins = 0
         local label = ui.text.new("Cell:")
         row:add_child(label)
         row:add_child(cell_picker)
         panel:add_child(row)
      end
      do
         local row   = ui.widget.new()
         row:set_layout("ltr")
         row.layout_margins = 0
         local label = ui.text.new("Navmesh:")
         row:add_child(label)
         row:add_child(navm_picker)
         panel:add_child(row)
      end
      panel:add_child(go_button)
      go_button.text = "Render"
      
      panel:add_spacer("v")
   end
   window:add_child(progress, 2, 1, 1, 2)
end

function clear_canvas()
   canvas.width  = 1
   canvas.height = 1
   local layers = {}
   do
      local source = canvas.layers
      local count  = #source
      for i = 1, count do
         layers[i] = source[i]
      end
   end
   for i = 1, #layers do
      canvas:remove_layer(layers[i])
   end
end
function repaint_canvas(zoom)
   clear_canvas()
   
   local navmesh = get_selected_navmesh()
   if not navmesh then
      return
   end
   local min_x = nil
   local max_x = nil
   local min_y = nil
   local max_y = nil
   do
      local list = navmesh.vertices
      local size = #list
      update_progress("Measuring navmesh bounds... (%v/%m vertices)", 0, size, 0)
      for i = 1, size do
         update_progress(nil, nil, nil, i)
         local v = list[i]
         local x = v.x
         local y = v.y
         if not min_x then
            min_x = x
            max_x = x
            min_y = y
            max_y = y
         else
            if x < min_x then min_x = x end
            if y < min_y then min_y = y end
            if x > max_x then max_x = x end
            if y > max_y then max_y = y end
         end
      end
   end
   if not min_x then
      return
   end
   
   local MARGIN = 5
   
   min_x = math.floor(min_x * zoom) - MARGIN
   min_y = math.floor(min_y * zoom) - MARGIN
   max_x = math.ceil(max_x * zoom) + MARGIN
   max_y = math.ceil(max_y * zoom) + MARGIN
   
   local width  = max_x - min_x
   local height = max_y - min_y
   canvas.width  = width
   canvas.height = height
   
   local layer  = canvas:append_layer()
   local raster = raster.new({
      width            = width,
      height           = height,
      background_color = "#FFF",
   })
   layer.data = raster
   
   do
      local offset = { x = min_x, y = min_y }
   
      local list = navmesh.triangles
      local size = #list
      update_progress("Painting triangles... (%v/%m)", 0, size, 0)
      for i = 1, size do
         update_progress(nil, nil, nil, i)
         local tri   = list[i]
         local verts = tri.vertices
         
         local a = vector2.new(verts[1])
         local b = vector2.new(verts[2])
         local c = vector2.new(verts[3])
         
         a = (a * zoom) - offset
         b = (b * zoom) - offset
         c = (c * zoom) - offset
--print(string.format("drawing triangle: (%f, %f) -> (%f, %f) -> (%f, %f)", a.x, a.y, b.x, b.y, c.x, c.y))
         
         local path = raster_draw_path.new()
         path:move_to(a)
         path:line_to(b)
         path:line_to(c)
         path:line_to(a)
         raster:draw_path({
            fill_color = "#FF8888",
            line_color = "#F00",
            path       = path,
         })
      end
   end
   layer.data = raster
end

go_button:on("OnActivated", "render", function()
   panel.enabled = false
   ui.run_when_unlocked(function()
      clear_canvas()
      repaint_canvas(1)
      panel.enabled = true
      clear_progress()
   end)
end)

window:show()
cell_picker:on("OnChanged", "update navmesh picker", function(index)
   rebuild_navmesh_picker(get_selected_cell())
end)
rebuild_cell_picker()