/*
 * File: mdb_page.h
 * Description: MiniDB Page interface
 * Author: 
 * E-mail:
 *
 */
#pragma once

class MDB_Page
{
    private:
        int page_id;  // unique identifier of the page
        char *page_data_ptr;   // pointer to page data
    public:
        MDB_Page();
        ~MDB_Page();
        void getData(char *&data_prt) const;
        void getPageID(int pg_id) const;
};
