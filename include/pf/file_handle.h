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
   int numPages;      // # of pages in the file
};

//
// PageFileHandle: PF File interface
//
class PageFileHandle
{
    private:
        BufferMgr *buffer_mgr_ptr;
        FileHeader hdr;
        int file_open;
        int if_dirty;
        int sysfd;
    public:
        PF_FileHandle();
        ~PF_FileHandle();
        void getFirstPage(MDB_Page &mdb_page) const;
        void getNextPage(int cur_pg_id, MDB_Page &mdb_page) const;
        void getPrevPage(int cur_pg_id, MDB_Page &mdb_page) const;
        void getThisPage(int pg_id, MDB_Page &mdb_page) const;
        void getLastPage(MDB_Page &mdb_page) const;
        void allocatePage(MDB_Page &mdb_page);
        void disposePage(int pg_id);
        void markDirty(int pg_id);
        void unpinPage(int pg_id);
        // Flush pages into disk from buffer pool
        void flushPages() const;
        // Write a page or pages to disk, but do not remove from buffer pool
        void writePages(int pg_id = -1);    // '-1' means all pages
};
