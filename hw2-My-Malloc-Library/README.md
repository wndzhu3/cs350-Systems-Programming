## Homework 2: My Malloc Library

This assignment implements `my_malloc` and associated functions which, when called, allocates `size` bytes of memory. I manipulate C pointers to traverse a process' address space, use pointer arithmetic to adjust pointer references, use casting to dereference memory storage as different types, and manually adjust the process heap. I was not allowed to use any library or system called besides `sbrk()`.

**Function Descriptions**

- `void* my_malloc(uint32_t size)`: allocates `size` bytes of memory.
- `void my_free(void *ptr)`: deallocates memory referenced by `ptr`, previously allocated by `my_malloc()`.
- `coalesce_free_list(void)`: merges logically adjacent chunks on the free list into single larger chunks.
- `FreeListNode free_list_begin(void)`: retrieves the first node of the free list.
