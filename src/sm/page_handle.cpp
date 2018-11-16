/*
 * File: page_handle.cpp
 * Description: MiniDB page handle implemention
 * Author: Liu Chaoyang
 * E-mail: chaoyanglius@gmail.com
 *
 */
#include <cstdio>
#include <iostream>
#include "sm/page_handle.h"

using std::cout;
using std::endl;

PageHandle::PageHandle()
{
    // -1 means invalid page
    page_id = -1;
    page_data_ptr = NULL;
}

PageHandle::PageHandle(const PageHandle &page_handle)
{
    page_id = page_handle.page_id;
    page_data_ptr = page_handle.page_data_ptr;
}

PageHandle::~PageHandle()
{
}

bool PageHandle::getData(char *&data_ptr) const
{
    data_ptr = NULL;
    // page can not be null
    if (page_data_ptr == NULL)
    {
        cout << "Error: page not in memory!" << endl;
        return false;
    }

    data_ptr = page_data_ptr;
}

bool PageHandle::getPageID(int &pg_id) const
{
    pg_id = -1; 
    // page can not be null
    if (page_data_ptr == NULL)
    {
        cout << "Error: page not in memory!" << endl;
        return false;
    }
    pg_id = page_id;
}

