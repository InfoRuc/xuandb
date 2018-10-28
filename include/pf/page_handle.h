/*
 * File: page_handle.h
 * Description: MiniDB Page interface
 * Author: 
 * E-mail:
 *
 */
#pragma once

class PF_PageHandle
{
    private:
        int page_id;  // unique identifier of the page
        char *page_data_ptr;   // pointer to page data
    public:
        PF_PageHandle();
        ~PF_PageHandle();
        void getData(char *&data_prt) const;
        void getPageID(int pg_id) const;
};
