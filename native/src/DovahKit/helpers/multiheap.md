
# `cobb::multiheap`

This is a multi-threaded block heap, allocating memory for several instances of a class at a time and then parcelling out that memory, instead of letting instances be `malloc`'d one at a time.

Internally, this heap is split into multiple sub-heaps. When a thread attempts to allocate memory, a new sub-heap is created for it; when the thread dies, the sub-heap is automatically destroyed, and its blocks are moved to a list of unowned blocks. (This is accomplished using a `thread_local` handle that uses RAII.) The effect of this is that allocations don't cause the entire heap to lock; lock contention only occurs when adding or removing threads, or when freeing any allocated object.

To help prevent memory fragmentation, if a sub-heap is created while there are unowned blocks, those blocks are assigned to the new sub-heap.[^potential-fragmentation-fixes]

[^potential-fragmentation-fixes]: This is not an optimal remedy. A more ideal fix would be to transfer ownership of just the first unowned block with any unused slots, and then make it so that when a sub-heap fills a block, it first asks to be given ownership of another unowned block with unused slots, only creating a new block for itself if no such unowned blocks exist. This would result in more locking during allocations, but only roughly every *n*th allocation given a block size of *n*.

The thread management happens entirely within `cobb::multiheap` using a singleton, which means that it's safe to use with *AllocatorAwareContainer* classes like `std::map`. Those classes will use `std::allocator::rebind` to, in effect, discard the allocator you actually give them and create a new one (inaccessible to the outside) templated on their internal node types. However, because you never actually need to access the heap from the outside (i.e. you don't need your threads to manually register with it), this isn't a problem. (Allocator rebinding does mean, however, that methods like (`force_free_all`) are unusable when working with STL containers.)

To that end, an interface is already available for allocator-aware containers: `cobb::multiheap_allocator`.

## Structure of the multiheap

| Nested class | Description |
| :- | :- |
| `block_info` | A header for each block, used to link blocks together (as a doubly-linked list) and track which slots in the block are currently in use. |
| `block_t` | A block of memory allocated from the application heap. This contains a header (`block_info`) and a buffer, the latter of which is divided and parcelled out to store instances of the mapped type. |
| `subheap` | A per-thread "sub-heap" which owns at least one block. |
| `subheap_handle` | A `thread_local` variable used as a handle. When a thread wants to allocate instances of the mapped type, it does so through this handle. The handle creates a sub-heap on the first attempted allocation, confers access to that sub-heap thereafter, and destroys the sub-heap in its own destructor (i.e. when the thread exits). |
| `State` | A singleton (per template instantiation) which owns all extant sub-heaps and, potentially, any unowned `block_t`s left behind by destroyed sub-heaps. |

Allocations route through a thread's `subheap_handle` to its `subheap` (creating that sub-heap in the process if it doesn't yet exist). Sub-heaps are registered with the `State`.

Frees route through the `State`, which checks all existing sub-heaps to see if any of them own the to-be-freed object's containing block. If not, the `State` then checks the unowned blocks.

## Locks

<dl>
   <dt><code>State::subheaps.lock</code></dt>
      <dd>
         <p>Lock for the <code>State</code>'s list of existing sub-heaps. The lock is acquired whenever the list is accessed, i.e. when a sub-heap is created or destroyed, or when an allocated object is freed.</p>
      </dd>
   <dt><code>State::unowned_blocks.lock</code></dt>
      <dd>
         <p>Lock for the <code>State</code>'s list of unowned blocks. The lock is acquired whenever the list is accessed, i.e. when a sub-heap is created or destroyed, or when an allocated object is freed.</p>
      </dd>
   <dt><code>subheap::alloc_free_lock</code></dt>
      <dd>
         <p>A lock acquired whenever the sub-heap allocates memory or is asked to free memory. This will be a low-contention lock; the only time multiple threads will ever try to acquire it is if the sub-heap's owning thread allocates an object at roughly the same time that another thread tries to free an object.</p>
      </dd>
</dl>