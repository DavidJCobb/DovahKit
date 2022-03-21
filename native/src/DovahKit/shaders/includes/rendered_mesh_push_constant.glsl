layout(push_constant) uniform PER_OBJECT {
   int   object_index;
	int   texture_index;
   int   texture_normal_index;
   float alpha_test_threshold;
   int   alpha_test_operation;
   int   enable_alpha_blending; // VkBool32
   int   receive_shadows; // VkBool32
} pushed;