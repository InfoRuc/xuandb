/*
 * File: buffer_mgr.cpp
 * Description: buffer manager implemention
 * Author: pinyin of your name
 * E-mail:
 *
 */
#include <cstring>
#include <climits>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include "sm/buffer_mgr.h"
#include "common.h"

using std::cout;
using std::endl;

BufferMgr::BufferMgr(int pg_num) : page_ht(), page_size(PAGE_WHOLE_SIZE), page_num(pg_num)
{
    // allocate memory for buffer page table
    buf_table = new BufPage[page_num]; 
    
    // init buffer table
    for (int i = 0; i < page_num; i++)
    {
        buf_table[i].data_ptr = new char[page_size];
        // TODO: add codes to check if allocate success
        memset((void *)buf_table[i].data_ptr, 0, page_size);
        // init the free page list, and free page list is full
        buf_table[i].prev = i - 1;
        buf_table[i].next = i + 1;
    }
    buf_table[0].prev = buf_table[page_num - 1].next = -1;
    // make free list head to first page
    free = 0;
    // used list is empty
    first = last = -1;
}

BufferMgr::~BufferMgr()
{
    // free buffer pages and its table
    for (int i = 0; i < page_num; i++)
        delete [] buf_table[i].data_ptr;
    delete [] buf_table;
}

//////////////////////////////////////////////////////////////
// Function: freeListInsert
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
void BufferMgr::freeListInsert(int slot)
{
    buf_table[slot].next = free;
    free = slot;
}

//////////////////////////////////////////////////////////////
// Function: usedListInsert
// Description: get a pointer to a page pinned in the buffer.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
void BufferMgr::usedListInsert(int slot)
{
    buf_table[slot].next = first;
    // make prev point to invalid slot
    buf_table[slot].prev = -1;
    
    // when list is not empty
    if (first != -1)
        buf_table[first].prev = slot;

    first = slot;

    // when list is empty
    if (last == -1)
        last = first;
}

//////////////////////////////////////////////////////////////
// Function: usedListRemove
// Description: 
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
void BufferMgr::usedListRemove(int slot)
{
    // if slot is at head of list
    if (first == slot)
        first = buf_table[slot].next;
    // if slot is at end of list
    if (last == slot)
        last = buf_table[slot].prev;
    // if slot is not at end of list
    if (buf_table[slot].next != -1)
        buf_table[buf_table[slot].next].prev = buf_table[slot].prev;
    
    // if slot is not at head of list
    if (buf_table[slot].prev != -1)
        buf_table[buf_table[slot].prev].next = buf_table[slot].next;

    // Set next and prev pointers of slot entry
    buf_table[slot].prev = buf_table[slot].next = -1;
}

//////////////////////////////////////////////////////////////
// Function: allocBufSlot
// Description: allocate a buffer slot
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::allocBufSlot(int &slot)
{
    if (free != -1)
    {
        slot = free;
        free = buf_table[slot].next;
    }
    else
    {
        // find the LRU and unpinned page
        for (slot = last; slot != -1; slot = buf_table[slot].prev)
        {
            if (buf_table[slot].pin_count == 0)
                break;
        }
        
        // return false if pages are unpined
        if (slot == -1)
            return false;

        // write page to disk if is dirty
        if (buf_table[slot].if_dirty)
        {
            if (!writePage(buf_table[slot].fd, buf_table[slot].page_id, buf_table[slot].data_ptr))
                return false;
            buf_table[slot].if_dirty = false;
        }

        // remove page from the hash table and slot from used list
        if (!page_ht.remove(buf_table[slot].fd, buf_table[slot].page_id))
            return false;
        else
            usedListRemove(slot);
    }
    // insert slot at the head of the used list
    usedListInsert(slot);

    return true;
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
    // search in buffer pool
    bool found = page_ht.search(fd, page_id, slot);
    // page is in buffer
    if (found)
    {
#ifdef DEBUG
        cout << "[BufferMgr Debug]: Page #"<< page_id << " is found in buffer." << endl;
#endif
        buf_table[slot].pin_count++;
        // make the page the mostly recently used
        usedListRemove(slot);
        usedListInsert(slot);
    }
    // page is not in buffer
    else
    {
#ifdef DEBUG
        cout << "[BufferMgr Debug]: Page #"<< page_id << " is not found in buffer." << endl;
        cout << "[BufferMgr Debug]: Loading page #" << page_id << " from disk..." << endl;
#endif
        // allocate an empty page and make it to MRU slot
        if (!allocBufSlot(slot))
        {
            cout << "[BufferMgr Error]: " << "Buffer pool full and no free page to exchange!" << endl;
            return false;
        }

        // init the page and pin the page
        initPageDesc(fd, page_id, slot);
        // read page from disk
        if (!readPage(fd, page_id, buf_table[slot].data_ptr) || 
            // insert page to buffer pool
            !page_ht.insert(fd, page_id, slot))
        {
            // read page failed, put the slot on the free list
            usedListRemove(slot);
            freeListInsert(slot);
            cout << "[BufferMgr Error]: Page #" << page_id << " loading failed!" << endl;
            return false;
        }
#ifdef DEBUG
        cout << "[BufferMgr Debug]: Page #" << page_id << " loading succeeded." << endl;
#endif
    }
    // get data pointer to its position in buffer pool
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

    // page is in buffer, failed
    if (!found)
        return false;

    // allocate a slot in buffer pool
    if (!allocBufSlot(slot))
        return false;

    // insert page into page hash table
    page_ht.insert(fd, page_id, slot);
    initPageDesc(fd, page_id, slot);

    // put the slot on the free list
    usedListRemove(slot);
    freeListInsert(slot);
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
    {
        cout << "[BufferMgr Error]: " << "#" << page_id << " is not in buffer!" << endl;
        return false;
    }
    
    // page unpin
    if (buf_table[slot].pin_count == 0)
    {
        cout << "[BufferMgr Error]: " << "#" << page_id << " has been unpined!" << endl;
        return false;
    }
    
    // mark the page dirty
    buf_table[slot].if_dirty = true;
    
    // make the page the MRU page
    usedListRemove(slot);
    usedListInsert(slot);
    
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
    {
        cout << "[BufferMgr Error]: " << "#" << page_id << " is not in buffer!" << endl;
        return false;
    }

    // page unpin
    if (buf_table[slot].pin_count == 0)
    {
        cout << "[BufferMgr Error]: " << "#" << page_id << " has been unpined!" << endl;
        return false;
    }

    // if is last pin, make it MRU page
    buf_table[slot].pin_count--;
    if ((buf_table[slot].pin_count) == 0)
    {
        usedListRemove(slot);
        usedListInsert(slot);
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
      if (buf_table[slot].fd == fd)
      {
         // ensure all pages are unpined
         if (!buf_table[slot].pin_count)
         {
             // only if page is dirty do write to disk
             if (buf_table[slot].if_dirty)
             {
#ifdef DEBUG
                cout << "[BufferMgr Debug]: #" << buf_table[slot].page_id << " page is dirty." << endl;
                cout << "[BufferMgr Debug]: Writing #" << buf_table[slot].page_id << " page..." << endl;
#endif 
                if (!writePage(fd, buf_table[slot].page_id, buf_table[slot].data_ptr))
                {
                    cout << "[BufferMgr Error]: " << "Writing #" << buf_table[slot].page_id << " page failed!" << endl;
                    return false;
                }
                buf_table[slot].if_dirty = false;
             }
             page_ht.remove(fd, buf_table[slot].page_id);
             usedListRemove(slot);
             freeListInsert(slot);
         }
         else
         {
#ifdef DEBUG
            cout << "[BufferMgr Debug]: "<< "#" << buf_table[slot].page_id << " page are pinned." << endl;
#endif 
         }
      }
      else
      {
         cout << "[BufferMgr Error]: " << "Wrong file decriptor!" << endl;
         return false;
      }
      slot = next;
   }
   return true; 
}

//////////////////////////////////////////////////////////////
// Function: clearBuffer
// Description: wrong
// back to disk
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::forcePage(int fd, int page_id)
{
    int slot = first;
    while (slot != -1)
    {
      int next = buf_table[slot].next;

      // If the page belongs to the passed-in file descriptor
      if (buf_table[slot].fd == fd && buf_table[slot].page_id == page_id)
      {
         // just write it when it is dirty.
         if (buf_table[slot].if_dirty) 
         {
#ifdef DEBUG
            cout << "[BufferMgr Debug]: #" << page_id << " page is dirty." << endl;
            cout << "[BufferMgr Debug]: Writing #" << page_id << " page..." << endl;
#endif
            if (!writePage(fd, buf_table[slot].page_id, buf_table[slot].data_ptr))
            {
                cout << "[BufferMgr Error]: " << "Writing #" << page_id << " page failed!" << endl;
                return false;
            }
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
            // remove all pages in buffer pool
            page_ht.remove(buf_table[slot].fd, buf_table[slot].page_id);
            // make all pages into free list
            usedListRemove(slot);
            freeListInsert(slot);
            slot = next;
        }
    }
    return true;
}

///////////////////////////////////////////
// NOT IMPLEMENTED AT PRESENT
// ///////////////////////////////////////
bool BufferMgr::resizeBuffer(int new_size)
{
    // TODO: resize buffer size
}

//////////////////////////////////////////////////////////////
// Function: readPage
// Description: read page from file(disk) to buffer
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::readPage(int fd, int page_id, char *dest)
{
    // seek the page place to read
    long offset = page_id * page_size + FILE_HEADER_SIZE;
    if (lseek(fd, offset, SEEK_SET) < 0)
    {
        cout << "[BufferMgr Error]: Seek page failed." << endl;
        return false;
    }

    // read data from file
    int bytes_num = read(fd, dest, page_size);
#ifdef DEBUG
    cout << "[BufferMgr Debug]: Reading #" << page_id << " page..." << endl;
    cout << "[BufferMgr Debug]: Trying to read " << page_size << " bytes data..." << endl;
#endif
    if (bytes_num == -1)
    {
        perror("[BufferMgr Error]: Read page error");
        return false;
    }

    if (bytes_num == 0)
    {
        cout << "[BufferMgr Error]: Read arrived to EOF!" << endl;
        return false;
    }

    if (bytes_num < page_size)
    {
        cout << "[BufferMgr Error]: Read incomplete page only " << bytes_num << " bytes!"<< endl;
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////
// Function: writePage
// Description: write page to disk from buffer
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::writePage(int fd, int page_id, char *source)
{
    long offset = page_id * page_size + PAGE_WHOLE_SIZE;

    if (lseek(fd, offset, SEEK_SET) < 0)
        return false;

    // write to file
    int bytes_num = write(fd, source, page_size);
    if (bytes_num < page_size)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////
// Function: initPageDesc
// Description: init a newly-pinned page decription info
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
void BufferMgr::initPageDesc(int fd, int page_id, int slot)
{
    buf_table[slot].fd = fd;
    buf_table[slot].page_id = page_id;
    buf_table[slot].if_dirty = false;
    buf_table[slot].pin_count = 1;
}

//////////////////////////////////////////////////////////////
// Function: allocateBlock
// Description: allocate block for a page from buffer pool
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::allocateBlock(char *&buffer)
{
    // get an empty slot from the buffer pool
    int slot;
    if (!allocBufSlot(slot))
        return false;

    // generate a unique page id
    // TODO: design a function to generate id for page
    int page_id = (long)buf_table[slot].data_ptr % (long)INT_MAX;

    if (!page_ht.insert(-1, page_id, slot))
    {
        usedListRemove(slot);
        freeListInsert(slot);
        return false;
    }
    else
        initPageDesc(-1, page_id, slot);

    // allocate block from buffer pool
    buffer = buf_table[slot].data_ptr;

    return true;
}

//////////////////////////////////////////////////////////////
// Function: disposeBlock
// Description: free the block from buffer pool by page_id
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool BufferMgr::disposeBlock(int page_id)
{
    return unpinPage(-1, page_id);
}

//////////////////////////////////////////////////////////////
// Function: getBlockSize
// Description: get block size(=page size)
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
void BufferMgr::getBlockSize(int &size) const
{
    size = page_size;
}
