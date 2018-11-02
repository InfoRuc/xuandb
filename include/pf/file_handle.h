/*
 * File: file_handle.h
 * Description: File handle interface
 * Author:
 * E-mail:
 *
 */
#pragma once

struct FileHeader
{
    int first_free;
    int pages_num;
};

class BufferMgr;

//
// PageFileHandle: PF File interface
//
class PageFileHandle
{
    private:
        BufferMgr *buffer_mgr;
        FileHeader hdr;
        bool file_open;
        bool hdr_changed;
        int sys_fd;
    public:
        PageFileHandle();
        // copy constructor
        PageFileHandle(const PageFileHandle &file_handle);
        // assignment of PageFileHandle by overload '='
        PageFileHandle& operator=(const PageFileHandle &file_handle);
        ~PageFileHandle();
        // 
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
        bool forcePages(int pg_id = -1);    // '-1' means all pages
};
