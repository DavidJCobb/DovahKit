local window   = ui.window.new()
local picker01 = ui.formpicker.new()
local picker02 = ui.formpicker.new()
local picker03 = ui.formpicker.new()
local lineedit = ui.textbox.new()
window:set_layout("grid")
window:add_child(picker01)
window:add_child(picker02)
window:add_child(picker03)
window:add_child(lineedit)

picker02.form_types = form_types.actor_base
picker03.form_types = { form_types.light, form_types.ammo }

local text = ui.text.new("[last change here]")
window:add_child(text)

function _handler(form)
   if form then
      text.text = form.editor_id
   else
      text.text = "[no form]"
   end
end
picker01:on("OnChanged", "readout", _handler)
picker02:on("OnChanged", "readout", _handler)
picker03:on("OnChanged", "readout", _handler)

function _edithandler(t)
   ui.run_when_unlocked(function()
      text.text = lineedit.text
   end)
end
lineedit:on("OnKeyPressed", "readout", _edithandler)

window:show()