local window = ui.window.new()
local form01 = ui.formpicker.new()
local form02 = ui.formpicker.new()
local form03 = ui.formpicker.new()
local text   = ui.text.new("...")
form02.form_types = form_types.actor_base
form03.form_types = { form_types.light, form_types.container }

form01.allow_none = true
form02.allow_none = true
form03.allow_none = true

window:set_layout("grid")
window:add_child(form01)
window:add_child(form02)
window:add_child(form03)
window:add_child(text)

function _handler(form)
   if form then
      text.text = form.editor_id
   else
      text.text = "NONE"
   end
end
form01:on("OnChanged", "readout", _handler)
form02:on("OnChanged", "readout", _handler)
form03:on("OnChanged", "readout", _handler)

window:show()