
local actor = dovah.create_form(form_types.actor_base)
actor.editor_id = "aaaTESTActor"

local quest = dovah.create_form(form_types.quest)
quest.editor_id = "aaaTESTQuest"

local branch = dovah.create_form(form_types.dialogue_branch)
branch.editor_id = "aaaTESTBranch"

local topic = dovah.create_form(form_types.topic, { parent = branch })
topic.editor_id = "aaaTESTTopic"

local info = dovah.create_form(form_types.topic_info, { parent = topic })

info.conditions:insert()
do
   local cnd = info.conditions[1]
   cnd.run_on   = "subject"
   cnd.function_name = "GetIsID"
   cnd.parameters[0] = actor
   cnd.comparison.operator = "=="
   cnd.comparison.operand  = 1
end
