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
    int free_page;
    int file_id;
}

class BufferMgr;

//
// FileHdr: Header structure for files
//
struct PF_FileHdr {
   int firstFree;     // first free page in the linked list
   int numPages;      // number of pages in the file
};

//
// PageFileHandle: PF File interface
//
class PageFileHandle
{
    private:
        BufferMgr *buffer_mgr_ptr;
        FileHeader hdr;
        bool if_open;
        bool if_dirty;
        int sys_fd;
    public:
        PageFileHandle();
        // copy constructor
        PageFileHandle::PageFileHandle(const PageFileHandle &file_handle)
        // assignment of PageFileHandle by overload '='
        PageFileHandle& operator=(const PageFileHandle &file_handle);
        ~PF_FileHandle();
        // 
        bool getFirstPage(PageHandle &page_handle) const;
        bool getNextPage(int cur_pg_id, PageHandle &page_handle) const;
        bool getPrevPage(int cur_pg_id, PageHandle &page_handle) const;
        // get a page by its id and get its handle
        bool getThisPage(int pg_id, PageHandle &page_handle) const;
        bool getLastPage(PageHandle &page_handle) const;
        bool allocatePage(PageHandle &page_handle);
        bool disposePage(int pg_id);
        bool markDirty(int pg_id);
        bool unpinPage(int pg_id);
        // Flush pages into disk from buffer pool
        bool flushPages() const;
        // Write a page or pages to disk, but do not remove from buffer pool
        bool writePages(int pg_id = -1);    // '-1' means all pages
};
