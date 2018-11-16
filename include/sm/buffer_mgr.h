/*
 * File: buffer_mgr.h
 * Description: buffer manager interface
 * Author: pinyin of your name
 * E-mail:
 *
 */
#pragma once

#include "common.h"
#include "sm/page_hashtable.h"
// TODO: Not use system call for file read/write
// To use C FILE or C++ fstream
enum Segment_t
{
    DATA_SEG = 0,
    INDEX_SEG = 1,
    LONG_SEG = 2,
    ROLLBACK_SEG = 4,
    TEMP_SEG = 5
};

// BufPage - struct containing data about a page in the buffer
//
struct BufPage
{
    char *data_ptr;     // page contents
    int next;       // next in the linked list of buffer pages
    int prev;       // prev in the linked list of buffer pages
    bool if_dirty;  // TRUE if page is dirty
    int pin_count;  // pin count
    int page_id;      // page number for this page
    int fd;     // OS file descriptor of this page
};

//
// BufferMgr
// manageing the page buffer
//
class BufferMgr
{
    private:
        BufPage *buf_table;       // info on buffer pages
        PageHashTable page_ht;      // Hash table object
        int page_num;             // number of pages in the buffer
        int page_size;             // size of page in the buffer
        int first;                // head of used list, used as MRU page slot
        int last;                 // tail of used list, used as LRU page slot
        int free;                // head of free list
        // table of data segment page table
        char ds_pt[DATA_SEGMENT_SIZE / 8];
        char ix_pt[INDEX_SEGMENT_SIZE / 8];
        char ls_pt[LONG_SEGMENT_SIZE / 8];
        char rb_pt[ROLLBACK_SEGMENT_SIZE / 8];
        char ts_pt[TEMP_SEGMENT_SIZE / 8];

        void freeListInsert(int slot); // insert slot at head of free list
        void usedListInsert(int slot); // insert slot at head of used list
        void usedListRemove(int slot); // remove slot from used list
        bool allocBufSlot(int &slot); // get a slot to use
        // read a page
        bool  readPage(int fd, int page_id, char *dest);
        // write a page
        bool  writePage(int fd, int page_id, char *source);
        // init the page desc entry
        void initPageDesc (int fd, int page_id, int slot);
        // generate unique id
        bool generatePageID(int &page_id, Segment_t seg);
        // find the page location
        bool pageLocate(int &offset, Segment_t seg, PageID_t page_id);
    public:
        BufferMgr(int pages_num);
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
        bool forcePage(int fd, int page_id);
        // remove all entries from buffer manager
        bool clearBuffer();
        // resize the buffer
        bool resizeBuffer(int new_size);
        // get size of block that can be allocated
        void getBlockSize(int &size) const;
        // allocate a memory chunk that lives in buffer manager
        bool allocateBlock(char *&buffer);
        // dispose of a memory chunk managed by buffer manager
        bool disposeBlock(int page_id);
};
