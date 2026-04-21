
local actor = dovah.create_form(form_types.actor_base)
actor.editor_id = "aaaTESTActor"

local quest = dovah.create_form(form_types.quest)
quest.editor_id = "aaaTESTQuest"

local branch = dovah.create_form(form_types.dialogue_branch, { parent = quest })
branch.editor_id = "aaaTESTBranch"

local topic = dovah.create_form(form_types.topic, { parent = branch })
topic.editor_id = "aaaTESTTopic"

local info = dovah.create_form(form_types.topic_info, { parent = topic })

info.conditions:insert()
do
   local cnd = info.conditions[1]
   cnd.run_on   = "subject"
   cnd.function_name = "GetIsID"
   cnd.parameters[1] = actor
   cnd.comparison.operator = "=="
   cnd.comparison.operand  = 1
end

info.conditions:insert()
do
   local cnd = info.conditions[2]
   cnd:overwrite_with({
      run_on        = "subject",
      function_name = "GetIsID",
      parameters    = { actor },
      comparison    = {
         operator = "==",
         operand  = 1
      }
   })
end

info.conditions:insert()
do
   local cnd = info.conditions[3]
   cnd:assign({
      -- BUG: "treat nil as unchanged" doesn't apply transitively to nested tables
      --      so we can't e.g. assign only the operator and leave the operand 
      --      unchanged.
      comparison  = {
         operator = "!=",
         operand  = 1
      },
      is_or_linked = true
   })
end

-- BUG: `insert` does not pay attention to any value you pass in, yet
info.conditions:insert(info.conditions[2])
do
   local cnd = info.conditions[4]
   cnd:assign({
      is_or_linked = true
   })
end