//
//  my_malloc.c
//  Lab1: Malloc
//

#include <unistd.h>
#include <stddef.h>
//#include <stdio.h>
#include "my_malloc.h"

MyErrorNo my_errno = MYNOERROR;
void * heap_bottom = NULL;

int find_chunkSize(size_t size) {
  int result = size + 8;
  result += size % 8;
  return result;
}

FreeListNode list = NULL;

void add_node(void * head, uint32_t size) {
  FreeListNode node = (FreeListNode)head;
  node->size = size;

  if (list == NULL) {
    list = node;
    node->flink = NULL;
    return;
  } else if (list > node) {
    node->flink = list;
    list = node;
    return;
  }

  FreeListNode prev = list;
  while (prev->flink != NULL) {
    if (prev->flink > node) {
      node->flink = prev->flink;
      prev->flink = node;
      return;
    } else {
      prev = prev->flink;
    }
  }

  prev->flink = node;
  node->flink = NULL;
  return;
}

FreeListNode search_freeList(uint32_t size) {
  FreeListNode result = NULL;
  if (list == NULL) {
    return NULL;
  } else if (list->size >= CHUNKHEADERSIZE + size) {
    result = list;
    list = list->flink;
    return result;
  }

  FreeListNode prev = list;
  while (prev->flink != NULL) {
    if (prev->flink->size >= CHUNKHEADERSIZE + size) {
      break;
    } else {
      prev = prev->flink;
    }
  }

  if (prev->flink != NULL) {
    result = prev->flink;
    prev->flink = result->flink;
    return result;
  } else {
    return NULL;
  }
}

void * my_malloc(uint32_t size){
  FreeListNode node = NULL; // usable_node
  void * head = NULL; // chunk_head
  uint32_t size_actual = 0; // actual_size

  if (heap_bottom == NULL) {
    heap_bottom = sbrk(0);
  }

  if (size % 8) {
    size += (8 - size % 8);
  }

  uint32_t roundedfreelistnode = sizeof(struct freelistnode);
  if (roundedfreelistnode % 8) {
    roundedfreelistnode += (8 - roundedfreelistnode % 8);
  }

  if (16 < roundedfreelistnode - CHUNKHEADERSIZE) {
    if (size < roundedfreelistnode - CHUNKHEADERSIZE) {
      size = roundedfreelistnode - CHUNKHEADERSIZE;
    }
  } else {
    if (size < 16) {
      size = 16;
    }
  }

  if ((node = search_freeList(size)) == NULL) {
    if (size <= 8192 - CHUNKHEADERSIZE) {
      if ((head = sbrk(8192)) == (void *)-1) {
        my_errno = MYENOMEM;
        return NULL;
      }
      uint32_t remainder_size = 8192 - size - CHUNKHEADERSIZE;
      if (16 < roundedfreelistnode && remainder_size >= roundedfreelistnode) {
        void * second_chunk_head = head + CHUNKHEADERSIZE + size;
        add_node(second_chunk_head, remainder_size);
        size_actual = size + CHUNKHEADERSIZE;
      } else if (16 > roundedfreelistnode && remainder_size >= 16) {
        void * second_chunk_head = head + CHUNKHEADERSIZE + size;
        add_node(second_chunk_head, remainder_size);
        size_actual = size + CHUNKHEADERSIZE;
      } else {
        size_actual = 8192;
      }
    } else {
      if ((head = sbrk(size + CHUNKHEADERSIZE)) == (void *)-1) {
        my_errno = MYENOMEM;
        return NULL;
      }
      size_actual = size + CHUNKHEADERSIZE;
    }
  } else {
    head = (void *)node;
    uint32_t remainder_size = node->size - size - CHUNKHEADERSIZE;
    if (16 < roundedfreelistnode && remainder_size >= roundedfreelistnode) {
      void * second_chunk_head = head + CHUNKHEADERSIZE + size;
      add_node(second_chunk_head, remainder_size);
      size_actual = size + CHUNKHEADERSIZE;
    } else if (16 > roundedfreelistnode && remainder_size >= 16) {
      void * second_chunk_head = head + CHUNKHEADERSIZE + size;
      add_node(second_chunk_head, remainder_size);
      size_actual = size + CHUNKHEADERSIZE;
    } else {
      size_actual = node->size;
    }
  }

  uint32_t * p_head = (uint32_t *)head;
  *p_head++ = size_actual;
  *p_head = 0xabcdef27;

  my_errno = MYNOERROR;
  return head + CHUNKHEADERSIZE;
}

void my_free(void *ptr){
  if (heap_bottom == NULL || ptr - CHUNKHEADERSIZE < heap_bottom
            || ptr > sbrk(0)) {
        my_errno = MYBADFREEPTR;
        return;
    }
    ptr -= CHUNKHEADERSIZE;
    uint32_t * p_head = (uint32_t *) ptr;
    if (*++p_head != 0xabcdef27) {
        my_errno = MYBADFREEPTR;
        return;
    }
    add_node(ptr, *--p_head);
    my_errno = MYNOERROR;
}

// return pointer to first chunk in free list
FreeListNode free_list_begin(){
  return list;
}

// merge adjacent chunks on the free list
void coalesce_free_list(){
  FreeListNode node = list;
  while (node != NULL) {
      if ((void *)node + node->size == node->flink) {
          node->size += node->flink->size;
          node->flink = node->flink->flink;
      } else {
          node = node->flink;
      }
  }
}
