/*
 * File: buffer_mgr.cpp
 * Description: buffer manager implemention
 * Author: pinyin of your name
 * E-mail:
 *
 */
#include "pf/page_hashtable.h"
#include "pf/buffer_mgr.h"

//////////////////////////////////////////////////////////////
// Function: insertFree
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::insertFree(int slot)
{

}

//////////////////////////////////////////////////////////////
// Function: linkHead
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::linkHead(int slot)
{

}

//////////////////////////////////////////////////////////////
// Function: unlink
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::unlink(int slot)
{

}

//////////////////////////////////////////////////////////////
// Function: unlink
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::internalAlloc(int &slot)
{

}

//////////////////////////////////////////////////////////////
// Function: getPage
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::getPage(int fd, int page_id, char *&buffer_ptr, bool multi_pined)
{
    int slot;
    bool found = page_ht.search(fd, page_id, slot);

    // page is in buffer
    if (!found)
    {
        buf_table->pin_count++;
        // make the page the mostly recently used
        if (!unlink(slot) || !linkHead(slot))
            return false;
    }
    // page is not in buffer
    else
    {
        // allocate an empty page and make it to MRU slot
        if (!internalAlloc(slot))
            return false;
        readPage(fd, page_id, buf_table[slot].data_ptr);
        page_ht.insert(fd, page_id, slot);
        initPageDesc(fd, page_id, slot);

        // put the slot on the free list
        unlink(slot);
        insertFree(slot);
    }
    buffer_ptr = buf_table[slot].data_ptr;
    return true;
}

//////////////////////////////////////////////////////////////
// Function: allocatePage
// Description: allocate a new page in the buffer
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::allocatePage(int fd, int page_id, char *&buffer_ptr)
{
    int slot;
    bool found = page_ht.search(fd, page_id, slot);

    if (!found)
        return false;

    // page is in buffer
    // allocate an empty page and make it to MRU slot
    if (!internalAlloc(slot))
        return false;
    page_ht.insert(fd, page_id, slot);
    initPageDesc(fd, page_id, slot);

    // put the slot on the free list
    unlink(slot);
    insertFree(slot);
    buffer_ptr = buf_table[slot].data_ptr;
    return true;
}

//////////////////////////////////////////////////////////////
// Function: markDirty
// Description: mark a page dirty so that it must be written
// back to disk
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::markDirty(int fd, int page_id)
{
    int slot;
    bool found = page_ht.search(fd, page_id, slot);

    if (!found)
        return false;
    
    // page unpin
    if (buf_table[slot].pin_count == 0)
        return false;
    
    // mark the page dirty
    buf_table[slot].if_dirty = true;
    
    // make the page the MRU page
    unlink(slot);
    linkHead(slot);
    
    return true;
}

//////////////////////////////////////////////////////////////
// Function: unpinPage
// Description: mark a page dirty so that it must be written
// back to disk
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::unpinPage(int fd, int page_id)
{
    int slot;
    bool found = page_ht.search(fd, page_id, slot);

    if (!found)
        return false;

    // page unpin
    if (buf_table[slot].pin_count == 0)
        return false;

    // if is last pin, make it MRU page
    if ((buf_table[slot].pin_count - 1) == 0)
    {
        unlink(slot);
        linkHead(slot);
    }
    return true;
}

//////////////////////////////////////////////////////////////
// Function: flushPages
// Description: mark a page dirty so that it must be written
// back to disk
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::flushPages(int fd)
{
    int slot = first;
    // when slot is valid
    while (slot != -1)
    {
      int next = buf_table[slot].next;

      // If the page belongs to the passed-in file descriptor
      if (buf_table[slot].sys_fd == fd)
      {
         // ensure all pages are unpined
         if (!buf_table[slot].pin_count)
         {
             // only if page is dirty do write to disk
             if (buf_table[slot].if_dirty)
             {
                writePage(fd, buf_table[slot].page_id, buf_table[slot].data_ptr);
                buf_table[slot].if_dirty = false;
             }
             page_ht.remove(fd, buf_table[slot].page_id);
             unlink(slot);
             insertFree(slot);
         }
         else
         {
             return false;
         }
      }
      slot = next;
   }
   return true; 
}

//////////////////////////////////////////////////////////////
// Function: clearBuffer
// Description: mark a page dirty so that it must be written
// back to disk
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::forcePages(int fd, int page_id)
{
    int slot = first;
    while (slot != -1)
    {
      int next = buf_table[slot].next;

      // If the page belongs to the passed-in file descriptor
      if (buf_table[slot].sys_fd == fd && buf_table[slot].page_id == page_id)
      {
         // just write it when it is dirty.
         if (buf_table[slot].if_dirty) 
         {
            writePage(fd, buf_table[slot].page_id, buf_table[slot].data_ptr);
            buf_table[slot].if_dirty = false;
         }
      }
      slot = next;
   }
   return true; 
}

//////////////////////////////////////////////////////////////
// Function: clearBuffer
// Description: mark a page dirty so that it must be written
// back to disk
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::clearBuffer()
{
    int slot, next;
    slot = first;

    while (slot != -1)
    {
        next = buf_table[slot].next;
        if (buf_table[slot].pin_count == 0)
        {
            page_ht.remove(buf_table[slot].sys_fd, buf_table[slot].page_id);
            unlink(slot);
            insertFree(slot);
            slot = next;
        }
    }
    return true;
}
