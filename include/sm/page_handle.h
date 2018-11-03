/*
 * File: page_handle.h
 * Description: MiniDB Page interface
 * Author: Liu Chaoyang
 * E-mail: chaoyanglius@gmail.com
 *
 */
#pragma once

#define PAGE_LIST_END -1
#define PAGE_USED -2

typedef int PageHdr ;
//////////////// Page Structure ///////////////////
//     ________________
//    |___page header__| 4 Bytes
//    |                |
//    |                |
//    |      data      | 4092 Bytes
//    |                |
//    |                |
//    |________________|
///////////////////////////////////////////////////

class StorageMgr;

class PageHandle
{
    friend class StorageMgr;
    private:
        int page_id;  // unique identifier of the page
        char *page_data_ptr;   // pointer to page data
    public:
        PageHandle();
        PageHandle(const PageHandle &page_handle);
        ~PageHandle();
        bool getData(char *&data_prt) const;     // get page contents by pointer
        bool getPageID(int &pg_id) const;    // get page id
};
