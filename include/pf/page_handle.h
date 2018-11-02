/*
 * File: page_handle.h
 * Description: MiniDB Page interface
 * Author: Liu Chaoyang
 * E-mail: chaoyanglius@gmail.com
 *
 */
#pragma once

class PageHandle
{
    private:
        int page_id;  // unique identifier of the page
        char *page_data_ptr;   // pointer to page data
    public:
        PageHandle();
        PageHandle(const PageHandle &page_handle);
        ~PageHandle();
        void getData(char *&data_prt) const;     // get page contents by pointer
        void getPageID(int &pg_id) const;    // get page id
};
