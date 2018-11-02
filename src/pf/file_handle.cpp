/*
 * File: file_handle.cpp
 * Description: File handle implemention
 * Author:
 * E-mail:
 *
 */
#include "pf/file_handle.cpp"
using std::cout;
using std::endl;

PageFileHandle::PageFileHandle()
{
    buffer_mgr_ptr = nullptr;
    file_open = false;
}

PageFileHandle::PageFileHandle(const PageFileHandle &file_handle)
{
    buffer_mgr_ptr = file_handle.buffer_mgr_ptr;
    hdr = file_handle.hdr;
    if_open = file_handle.if_open;
    if_dirty = file_handle.if_dirty;
    sys_fd = file_handle.sys_fd;
}

PageFileHandle& PageFileHandle::operator=(const PageFileHandle &file_handle)
{
    buffer_mgr_ptr = file_handle.buffer_mgr_ptr;
    hdr = file_handle.hdr;
    if_open = file_handle.if_open;
    if_dirty = file_handle.if_dirty;
    sys_fd = file_handle.sys_fd;

    return *this;
}

bool PageFileHandle::getThisPage(int pg_id, PageHandle &page_handle)
{
    
}

bool PageFileHandle::getFirstPage(PageHandle &page_handle) const
{
    getNextPage((page_id) - 1, page_handle);
}

bool PageFileHandle::getNextPage(int cur_pg_id, PageHandle &page_handle) const
{
    if (!if_open)    
    {
        cout << "Error: File not open!" << endl;
        exit(EXIT_FAILURE);
    }

    if (cur_pg_id != -1)
    {
        cout << "Error: Page ID is invalid!" << endl;
        exit(EXIT_FAILURE);
    }

    // scan file until a valid used page is found
    for (int i = 0; i < )
}

bool PageFileHandle::getPrevPage(int cur_pg_id, PageHandle &page_handle) const
{

}
