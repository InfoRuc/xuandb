/*
 * File: buffer_mgr.h
 * Description: buffer manager interface
 * Author: pinyin of your name
 * E-mail:
 *
 */
#pragma once

//
// BufPageDesc - struct containing data about a page in the buffer
//
struct BufPageDesc
{
    char *data_ptr;     // page contents
    int next;       // next in the linked list of buffer pages
    int prev;       // prev in the linked list of buffer pages
    bool if_dirty;  // TRUE if page is dirty
    int pin_count;  // pin count
    int page_id;      // page number for this page
    int sys_fd;     // OS file descriptor of this page
};

class PageHashTable;
//
// BufferMgr
// manageing the page buffer
//
class BufferMgr
{
    private:
        BufPageDesc *buf_table;       // info on buffer pages
        PageHashTable page_ht;      // Hash table object
        int page_id;             // number of pages in the buffer
        int page_size;             // Size of pages in the buffer
        int first;                // MRU page slot
        int last;                 // LRU page slot
        int free;                // head of free list
        bool insertFree(int slot);     // Insert slot at head of free
        bool linkHead(int slot);       // Insert slot at head of used
        bool unlink(int slot);          // Unlink slot
        bool internalAlloc(int &slot);      // Get a slot to use
        // read a page
        bool  readPage(int fd, int page_id, char *dest);
        // write a page
        bool  writePage(int fd, int page_id, char *source);
        // init the page desc entry
        bool  initPageDesc (int fd, int page_id, int slot);
    public:
        BufferMgr();
        ~BufferMgr();
        // get a page from buffer
        bool getPage(int sys_fd, int page_id, char *&buffer_ptr, bool multi_pined = true);
        // allocate a new page in the buffer
        bool allocatePage(int fd, int page_id, char *&buffer_ptr);
        // mark page dirty
        bool markDirty(int fd, int page_id);
        // unpin page from buffer
        bool unpinPage(int fd, int page_id);
        // flush pages for file
        bool flushPages(int fd);
        // force a page to disk, but not removing from buffer
        bool forcePages(int fd, int page_id);
        // remove all entries from buffer manager
        bool clearBuffer();
        // resize the buffer
        bool resizeBuffer(int new_size);
        // get size of block that can be allocated
        bool getBlockSize(int &length) const;
        // allocate a memory chunk that lives in buffer manager
        bool allocateBlock(char *&buffer);
        // dispose of a memory chunk managed by buffer manager
        bool disposeBlock(char *buffer);
};
