
#ifndef INCLUDE_GUARD_VkDrawIndexedIndirectCommand // include guard
#define INCLUDE_GUARD_VkDrawIndexedIndirectCommand

struct VkDrawIndexedIndirectCommand {
   uint indexCount;
   uint instanceCount;
   uint firstIndex;
   uint vertexOffset;
   uint firstInstance;
};

#endif // include guard