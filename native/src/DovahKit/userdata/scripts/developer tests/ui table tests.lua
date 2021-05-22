local window = ui.window.new()
local table  = ui.table_view.new()

window:set_layout("down")
window:add_child(table)

table.column_headers = { "Quest", "Form ID", "Editor ID", "Info Text" }
table.selection_mode = "single"
table.show_row_headers = false
table.word_wrap = "truncate"

table:append_row("A", "B", "C", "D")
table:append_row("E", "F", "G", "H")
table:append_row("I", "J")
table:append_row_with_options(
   { text_color = "#FF00A0", text = "default" },
   "K", "L", nil, "M"
)

do
   local panel = ui.widget.new()
   panel:set_layout("grid")
   panel.layout_margins = 0
   
   do
      local st = ui.dropdown.new()
      st:append_item("Select Rows")
      st:append_item("Select Columns")
      st:append_item("Select Cells")
      st:on("OnChanged", "", function(i)
         if i == 1 then
            table.selection_type = "rows"
         elseif i == 2 then
            table.selection_type = "columns"
         elseif i == 3 then
            table.selection_type = "cells"
         end
      end)
      panel:add_child(st, 1, 1)
   end
   do
      local button = ui.button.new("Access Selection")
      button:on("OnActivated", "", function()
         local sel = table.selection
         if not sel then
            dovah.log_message("No selection")
            return
         end
         dovah.log_message("Selection: %s", tostring(sel))
         local st = table.selection_type
         if st == "rows" then
            for _, v in ipairs(sel) do
               local list = v.cells
               dovah.log_message("Row %s containing %d cells", tostring(v), #list)
               local text = nil
               for i = 1, #list do
                  local cell = list[i]
                  local item = ""
                  if cell then
                     item = tostring(cell.text)
                  else
                     item = "nil"
                  end
                  if text then
                     text = text .. " | " .. item
                  else
                     text = item
                  end
               end
               dovah.log_message(" - Cells: " .. text)
            end
         elseif st == "columns" then
            for _, v in ipairs(sel) do
               local list = v.cells
               dovah.log_message("Col %s containing %d cells", tostring(v), #list)
               local text = nil
               for i = 1, #list do
                  local cell = list[i]
                  local item = ""
                  if cell then
                     item = tostring(cell.text)
                  else
                     item = "nil"
                  end
                  if text then
                     text = text .. " | " .. item
                  else
                     text = item
                  end
               end
               dovah.log_message(" - Cells: " .. text)
            end
         elseif st == "cells" then
            for _, v in ipairs(sel) do
               dovah.log_message("Cell: %s", tostring(v))
               local r = v.row
               local c = v.column
               --
               local wr = table.rows[r]
               local wc = table.columns[c]
               dovah.log_message(" - Containing row: %s", tostring(wr))
               dovah.log_message(" - Containing col: %s", tostring(wc))
               if wr then
                  local eq = wr.cells[r] == v
                  dovah.log_message(" - Row[%d] == Cell? %s (%s)", r, tostring(eq), tostring(wr.cells[r]))
               end
               if wc then
                  local eq = wc.cells[c] == v
                  dovah.log_message(" - Col[%d] == Cell? %s (%s)", c, tostring(eq), tostring(wc.cells[c]))
               end
            end
         end
         dovah.log_message("Done printing selection")
      end)
      panel:add_child(button, 1, 2)
   end
   do
      local button = ui.button.new("Recolor Selection")
      button:on("OnActivated", "", function()
         local sel = table.selection
         if not sel then
            return
         end
         for _, v in ipairs(sel) do
            v.text_color = { 64, 192, 0 }
         end
      end)
      panel:add_child(button, 2, 2)
   end
   do
      local button = ui.button.new("Echo Selection Colors")
      button:on("OnActivated", "", function()
         local sel = table.selection
         if not sel then
            return
         end
         for _, v in ipairs(sel) do
            local tc = v.text_color
            dovah.log_message("R: %d | G: %d | B: %d | A: %d", tc.r, tc.g, tc.b, tc.a)
         end
      end)
      panel:add_child(button, 3, 2)
   end
   do
      local button = ui.button.new("Set Sel. Icon Color")
      button:on("OnActivated", "", function()
         local sel = table.selection
         if not sel then
            return
         end
         local c  = "hsl(0, 100%, 50%)"
         local st = table.selection_type
         --
         function _handle(cell)
            if cell then
               cell.icon = c
            end
         end
         --
         if st == "rows" then
            for _, v in ipairs(sel) do
               local list = v.cells
               for i = 1, #list do
                  _handle(list[i])
               end
            end
         elseif st == "columns" then
            for _, v in ipairs(sel) do
               local list = v.cells
               for i = 1, #list do
                  _handle(list[i])
               end
            end
         elseif st == "cells" then
            for _, v in ipairs(sel) do
               _handle(v)
            end
         end
      end)
      panel:add_child(button, 4, 1)
   end
   do
      local button = ui.button.new("Set Sel. Icon Raster")
      button:on("OnActivated", "", function()
         local sel = table.selection
         if not sel then
            return
         end
         local c  = "hsl(0, 100%, 50%)"
         local r  = raster.new({ width = 8, height = 8, background_color = c })
         local st = table.selection_type
         --
         function _handle(cell)
            if cell then
               cell.icon = r
            end
         end
         --
         if st == "rows" then
            for _, v in ipairs(sel) do
               local list = v.cells
               for i = 1, #list do
                  _handle(list[i])
               end
            end
         elseif st == "columns" then
            for _, v in ipairs(sel) do
               local list = v.cells
               for i = 1, #list do
                  _handle(list[i])
               end
            end
         elseif st == "cells" then
            for _, v in ipairs(sel) do
               _handle(v)
            end
         end
      end)
      panel:add_child(button, 4, 2)
   end
   
   window:add_child(panel)
end

window:show()