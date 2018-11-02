/*
 * File: page_handle.cpp
 * Description: MiniDB page handle implemention
 * Author: Liu Chaoyang
 * E-mail: chaoyanglius@gmail.com
 *
 */
#include "page_handle.h"

PageHandle::PageHandle()
{
    // -1 means invalid page
    page_id = -1;
    page_data_ptr = nullptr;
}

PageHandle::PageHandle(const PageHandle &page_handle)
{
    page_id = page_handle.page_id;
    page_data_ptr = page_handle.page_data_ptr;
}

PageHandle::~PageHandle()
{
}

void PageHandle::getData(char *&data_ptr) const
{
    data_ptr = nullptr;
    // page can not be null
    if (page_data_ptr == nullptr)
    {
        cout << "Error: page not in memory!" << endl;
        exit(EXIT_FAILURE);
    }

    data_ptr = page_data_ptr;
}

void PageHandle::getPageID(int &pg_id)
{
    pg_id = -1; 
    // page can not be null
    if (page_data_ptr == nullptr)
    {
        cout << "Error: page not in memory!" << endl;
        exit(EXIT_FAILURE);
    }
    pg_id = page_id;
}







