local OPTION_ALWAYS_USE_RASTERS = true
-- NOTE: Using to-be-closed variables requires the above option

local window = ui.window.new()
local picker = ui.dropdown.new()

local paths = {
   "textures/landscape/cavebaseground01.dds",
   "textures/landscape/cavebaseground01_n.dds",
   "textures/landscape/coastbeach01.dds",
   "textures/landscape/coastbeach01_n.dds",
   "textures/landscape/coastbeach02.dds",
   "textures/landscape/coastbeach02_n.dds",
   "textures/landscape/coastbeachgrass01.dds",
   "textures/landscape/coastbeachgrass01_n.dds",
   "textures/landscape/coastoceanfloor01.dds",
   "textures/landscape/coastoceanfloor01_n.dds",
   "textures/landscape/dirt01.dds",
   "textures/landscape/dirt01_n.dds",
   "textures/landscape/dirt02.dds",
   "textures/landscape/dirt02_n.dds",
   "textures/landscape/dirtcliffs/dirtcliffs01.dds",
   "textures/landscape/dirtcliffs/dirtcliffs01_n.dds",
   "textures/landscape/dirtcliffs/dirtcliffsroots01.dds",
   "textures/landscape/dirtcliffs/dirtcliffsroots01_n.dds",
   "textures/landscape/dirtpath01.dds",
   "textures/landscape/dirtpath01_n.dds",
   "textures/landscape/dirtsnowpath01.dds",
   "textures/landscape/dirtsnowpath01_n.dds",
   "textures/landscape/fallforestdirt01.dds",
   "textures/landscape/fallforestdirt01_n.dds",
   "textures/landscape/fallforestgrass01.dds",
   "textures/landscape/fallforestgrass01_n.dds",
   "textures/landscape/fallforestleaves01.dds",
   "textures/landscape/fallforestleaves01_n.dds",
   "textures/landscape/fallforestrocks01.dds",
   "textures/landscape/fallforestrocks01_n.dds",
   "textures/landscape/fielddirtgrass01.dds",
   "textures/landscape/fielddirtgrass01_n.dds",
   "textures/landscape/fieldgrass01.dds",
   "textures/landscape/fieldgrass01_n.dds",
   "textures/landscape/fieldgrass02.dds",
   "textures/landscape/fieldgrass02_n.dds",
   "textures/landscape/frozenmarshdirtslopes01.dds",
   "textures/landscape/frozenmarshdirtslopes01_n.dds",
   "textures/landscape/frozenmarshgrass01.dds",
   "textures/landscape/frozenmarshgrass01_n.dds",
   "textures/landscape/frozenmarshice01.dds",
   "textures/landscape/frozenmarshice01_n.dds",
   "textures/landscape/frozenmarshlichen01.dds",
   "textures/landscape/frozenmarshlichen01_n.dds",
   "textures/landscape/glacierenvironmentmask.dds",
   "textures/landscape/glacierparallax.dds",
   "textures/landscape/glacierslab.dds",
   "textures/landscape/glacierslab_n.dds",
   "textures/landscape/glaciersubsurface.dds",
   "textures/landscape/grass/coastbeachgrassobj01.dds",
   "textures/landscape/grass/coastcoralgrass01.dds",
   "textures/landscape/grass/coastkelpgrass01.dds",
   "textures/landscape/grass/coastkelpgrass01_n.dds",
   "textures/landscape/grass/fallforestgrassobj01.dds",
   "textures/landscape/grass/fallforestgrassobj02.dds",
   "textures/landscape/grass/fallforestgrassobj02_n.dds",
   "textures/landscape/grass/fieldgrassobj01.dds",
   "textures/landscape/grass/fieldgrassobj02.dds",
   "textures/landscape/grass/forestgrass01.dds",
   "textures/landscape/grass/forestgrass01_n.dds",
   "textures/landscape/grass/forestgrass02.dds",
   "textures/landscape/grass/frozenmarshgrassobj01.dds",
   "textures/landscape/grass/reachgrassobj01.dds",
   "textures/landscape/grass/reachgrassobj01_n.dds",
   "textures/landscape/grass/rocksgrass01.dds",
   "textures/landscape/grass/rocksgrasssnow01.dds",
   "textures/landscape/grass/rocksgrasswater01.dds",
   "textures/landscape/grass/snowgrass01.dds",
   "textures/landscape/grass/snowgrass02.dds",
   "textures/landscape/grass/swordfern.dds",
   "textures/landscape/grass/swordfern_n.dds",
   "textures/landscape/grass/tempfieldgrassobj01_n.dds",
   "textures/landscape/grass/tundragrassobj01.dds",
   "textures/landscape/grass/tundragrassobj02.dds",
   "textures/landscape/grass/tundragrassobj02_n.dds",
   "textures/landscape/grass/tundragrassobj03.dds",
   "textures/landscape/grass/tundragrassobj04.dds",
   "textures/landscape/grass/volcanictundragrassobj01.dds",
   "textures/landscape/grass/volcanictundragrassobj02.dds",
   "textures/landscape/grasssnow01.dds",
   "textures/landscape/grasssnow01_n.dds",
   "textures/landscape/icefloes.dds",
   "textures/landscape/icefloes_n.dds",
   "textures/landscape/icicle.dds",
   "textures/landscape/icicle_n.dds",
   "textures/landscape/icicleparallax.dds",
   "textures/landscape/mineralpoolterrace.dds",
   "textures/landscape/mineralpoolterrace_n.dds",
   "textures/landscape/mountains/mountainslab01.dds",
   "textures/landscape/mountains/mountainslab01_n.dds",
   "textures/landscape/mountains/mountainslab02.dds",
   "textures/landscape/mountains/mountainslab02_n.dds",
   "textures/landscape/pineforest01.dds",
   "textures/landscape/pineforest01_n.dds",
   "textures/landscape/pineforest02.dds",
   "textures/landscape/pineforest02_n.dds",
   "textures/landscape/pineforest03.dds",
   "textures/landscape/pineforest03_n.dds",
   "textures/landscape/reachdirt01.dds",
   "textures/landscape/reachdirt01_n.dds",
   "textures/landscape/reachgrass01.dds",
   "textures/landscape/reachgrass01_n.dds",
   "textures/landscape/reachmoss01.dds",
   "textures/landscape/reachmoss01_n.dds",
   "textures/landscape/reachmossyrocks01.dds",
   "textures/landscape/reachmossyrocks01_n.dds",
   "textures/landscape/riverbededge.dds",
   "textures/landscape/riverbededge_n.dds",
   "textures/landscape/riverbottom.dds",
   "textures/landscape/riverbottom_n.dds",
   "textures/landscape/rivermud.dds",
   "textures/landscape/rivermud_n.dds",
   "textures/landscape/roads/bridge01.dds",
   "textures/landscape/roads/bridge01_n.dds",
   "textures/landscape/roads/road01.dds",
   "textures/landscape/roads/road01_n.dds",
   "textures/landscape/roads/road01fallforest01.dds",
   "textures/landscape/roads/road01reach01.dds",
   "textures/landscape/roads/road01snow01.dds",
   "textures/landscape/roads/road01snow01_n.dds",
   "textures/landscape/roads/roaddetails01.dds",
   "textures/landscape/roads/roaddetails01_n.dds",
   "textures/landscape/rocks01.dds",
   "textures/landscape/rocks01_n.dds",
   "textures/landscape/rocksedgetrim01.dds",
   "textures/landscape/rocksedgetrim01_n.dds",
   "textures/landscape/rockset_n.dds",
   "textures/landscape/snow01.dds",
   "textures/landscape/snow01_n.dds",
   "textures/landscape/snow01alta.dds",
   "textures/landscape/snow02.dds",
   "textures/landscape/snow02_n.dds",
   "textures/landscape/snowrocks01.dds",
   "textures/landscape/snowrocks01_n.dds",
   "textures/landscape/snowstone01.dds",
   "textures/landscape/snowstone01_n.dds",
}

window:set_layout("grid")
window:add_child(picker, 1, 1)

do
   function _ends_with(haystack, needle)
      return haystack:sub(-#needle) == needle
   end
   
   for i = 1, #paths do
      local path = paths[i]
      if not _ends_with(path, "_n.dds") then
         local name = path -- TODO: get filename only
         local icon <close> = dovah.lookup_game_asset(paths[i])
         if icon and dovah.type(icon) == "raster" then
            picker:append_item({
               text = name,
               icon = icon
            })
         elseif icon then
            dovah.log_message("Not a raster: %s", paths[i])
         else
            dovah.log_message("Missing: %s", paths[i])
         end
      end
   end
end

do
   local sacrificial = raster.new({ width = 32, height = 32, background_color = "#000" })
   picker:append_item({
      text = "Testing Placeholder",
      icon = sacrificial
   })
   --
   local button = ui.button.new("Modify Placeholder")
   button:on("OnActivated", "", function()
      local x = math.random(1, 32)
      local y = math.random(1, 32)
      local hue = math.random(0, 359)
      sacrificial:set_pixel(x, y, "hsl(" .. hue .. "deg, 100%, 50%)")
   end)
   --
   window:add_child(button, 2, 1)
end

window:show()