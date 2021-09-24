
WATER_DEPTH     = 4096
WATER_MIN_ALPHA = 0.5

CELL_OUTLINE_FILL_OPACITY = 30 -- [0, 255]

LAYER_SPEC = {
   {
      name       = "paint",
      blend_mode = "multiply",
      opacity    = 0.65,
      check_text = "Show landscape paint",
   },
   {
      name       = "color",
      blend_mode = "multiply",
      check_text = "Show vertex colors",
   },
   {
      name       = "water",
      check_text = "Show water",
   },
}
