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

table:on("OnSelectionChanged", "", function(row, ...)
   if not row then
      dovah.log_message("No row selected")
      return
   end
   dovah.log_message(row)
   dovah.log_message(row.cells)
   local cell = row.cells[2]
   if not cell then
      dovah.log_message("Cell is missing")
      return
   end
   dovah.log_message(cell)
   dovah.log_message("Cell text: %s", cell.text)
end)

window:show()