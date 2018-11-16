/*
 * File: storage_mgr.h
 * Description: File handle interface
 * Author: Liu Chaoyang
 * E-mail: chaoyanglius@gmail.com
 *
 */
#pragma once

#include "common.h"

struct FileHeader
{
    int first_free;
    int pages_num;
    char ds_pt[DATA_SEGMENT_SIZE / 8];
    char ix_pt[INDEX_SEGMENT_SIZE / 8];
    char ls_pt[LONG_SEGMENT_SIZE / 8];
    char rb_pt[ROLLBACK_SEGMENT_SIZE / 8];
    char ts_pt[TEMP_SEGMENT_SIZE / 8];
};

class BufferMgr;
class PageHandle;
//
// Class: StorageMgr
// Description: manage disk and buffer. Using a big file to storage all record.
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//
class StorageMgr
{
    private:
        FileHeader hdr;
        bool file_open;
        bool hdr_changed;
        int sys_fd;
        BufferMgr *buffer_mgr;
    public:
        StorageMgr();
        // copy constructor
        StorageMgr(const StorageMgr &storage_mgr);
        // assignment of StorageMgr by overload '='
        StorageMgr& operator=(const StorageMgr &storage_mgr);
        ~StorageMgr();
        bool initSM(const char *file_name);
        bool getFirstPage(PageHandle &page_handle);
        bool getNextPage(int cur_pg_id, PageHandle &page_handle);
        bool getPrevPage(int cur_pg_id, PageHandle &page_handle);
        // get a page by its id and get its handle
        bool getThisPage(int pg_id, PageHandle &page_handle);
        bool getLastPage(PageHandle &page_handle);
        bool allocatePage(PageHandle &page_handle);
        bool disposePage(int pg_id);
        bool markDirty(int pg_id);
        bool unpinPage(int pg_id);
        // Flush pages into disk from buffer pool
        bool flushPages();
        // Write a page or pages to disk, but do not remove from buffer pool
        bool forcePage(int pg_id = -1);    // '-1' means all pages
};
