/*
 * File: file_handle.cpp
 * Description: File handle implemention
 * Author:
 * E-mail:
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include "common.h"
#include "pf/page_handle.h"
#include "pf/file_handle.h"
#include "pf/buffer_mgr.h"
#include "pf/page_hashtable.h"

using std::cout;
using std::endl;

PageFileHandle::PageFileHandle()
{
    buffer_mgr = NULL;
    file_open = false;
}

PageFileHandle::PageFileHandle(const PageFileHandle &file_handle)
{
    buffer_mgr = file_handle.buffer_mgr;
    hdr = file_handle.hdr;
    file_open = file_handle.file_open;
    hdr_changed = file_handle.hdr_changed;
    sys_fd = file_handle.sys_fd;
}

PageFileHandle& PageFileHandle::operator=(const PageFileHandle &file_handle)
{
    buffer_mgr = file_handle.buffer_mgr;
    hdr = file_handle.hdr;
    file_open = file_handle.file_open;
    hdr_changed = file_handle.hdr_changed;
    sys_fd = file_handle.sys_fd;

    return *this;
}

bool PageFileHandle::getThisPage(int page_id, PageHandle &page_handle) 
{
    char *page_buf_ptr;
    if (!file_open)
    {
        return false;
    }

    if (page_id > hdr.pages_num || page_id < 0)
    {
        return false;
    }

    // get page by page id
    if (!buffer_mgr->getPage(sys_fd, page_id, page_buf_ptr));
    {
        return false;
    }

    // if page is valid
    if (((PageHdr*)page_buf_ptr)[0] == PAGE_USED)
    {
        page_handle.page_id = page_id;
        page_handle.page_data_ptr = page_buf_ptr + sizeof(PageHdr);
        return true;
    }
    
    // if page is invalid
    if (!unpinPage(page_id))
    {
        return false;
    }

    return false;
}

bool PageFileHandle::getFirstPage(PageHandle &page_handle) 
{
    return getNextPage(-1, page_handle);
}

bool PageFileHandle::getLastPage(PageHandle &page_handle) 
{
    return getPrevPage(hdr.pages_num, page_handle);
}

bool PageFileHandle::getNextPage(int cur_pg_id, PageHandle &page_handle) 
{
    // file must be open
    if (!file_open)    
    {
        cout << "Error: File not open!" << endl;
        return false;
    }

    if (cur_pg_id != -1 && (cur_pg_id > hdr.pages_num || cur_pg_id < 0))
    {
        cout << "Error: Page ID is invalid!" << endl;
        return false;
    }

    // scan file until a valid used page is found
    for (cur_pg_id++; cur_pg_id < hdr.pages_num; cur_pg_id++)
    {
        if (getThisPage(cur_pg_id, page_handle))
        {
#ifdef DEBUG
            cout << "DEBUG: Valid page found." << endl;
#endif
            return true;
        }
        else
        {
            cout << "Error: Invalid page!" << endl;
            return false;
        }
    }

    cout << "Error: No valid page found!" << endl;
    return false;
}

bool PageFileHandle::getPrevPage(int cur_pg_id, PageHandle &page_handle) 
{
    // file must be open
    if (!file_open)    
    {
        cout << "Error: File not open!" << endl;
        return false;
    }

    // check if invalid
    if (cur_pg_id != hdr.pages_num && (cur_pg_id > hdr.pages_num || cur_pg_id < 0))
    {
        cout << "Error: Page ID is invalid!" << endl;
        return false;
    }

    // scan file until a valid used page is found
    for (cur_pg_id--; cur_pg_id < hdr.pages_num; cur_pg_id--)
    {
        if (getThisPage(cur_pg_id, page_handle))
            return true;
        else
        {
            cout << "Error: Invalid page!" << endl;
            return false;
        }
    }

    cout << "Error: No valid page found!" << endl;
    return false;
}

bool PageFileHandle::disposePage(int page_id)
{
    char *page_buf_ptr;

    // check if invalid
    if (page_id > hdr.pages_num || page_id < 0)
    {
        return false;
    }

    // get the page to be disposed
    if (!buffer_mgr->getPage(sys_fd, page_id, page_buf_ptr))
    {
        return false;
    }

    // if page is valid
    if (((PageHdr*)page_buf_ptr)[0] != PAGE_USED)
    {
        if (!unpinPage(page_id))
        {
            return false;
        }
        cout << "Error: Page already free!" << endl;
        return false;
    }
    
    // put the page into free list
    ((PageHdr*)page_buf_ptr)[0] = hdr.first_free;
    hdr.first_free = page_id;
    hdr_changed = true;
    
    // page has been dirty
    if (!markDirty(page_id))
    {
        return false;
    }

    // unpin the page
    if (!unpinPage(page_id))
    {
        return false;
    }

    return true;
}

bool PageFileHandle::allocatePage(PageHandle &page_handle)
{
    int page_id;    // new page id
    char *page_buf_ptr;

    if (!file_open)
    {
        return false;
    }

    // if free list is not empty
    if (hdr.first_free != -1)
    {
        page_id = hdr.first_free;

        // make the first free page into buffer
        if (!buffer_mgr->getPage(sys_fd, page_id, page_buf_ptr))
        {
            return false;
        }
        hdr.first_free = ((PageHdr*)page_buf_ptr)[0];
    }
    else
    {
        page_id = hdr.pages_num;

        // allocate new page
        if (!buffer_mgr->allocatePage(sys_fd, page_id, page_buf_ptr))
        {
            return false;
        }
        hdr.pages_num++;
    }

    hdr_changed = true;

    // mark the page used
    ((PageHdr*)page_buf_ptr)[0] = PAGE_USED;

    // zero out the page data
    memset(page_buf_ptr + sizeof(PageHdr), 0, PAGE_DATA_SIZE);

    // mark dirty
    if (!markDirty(page_id))
        return false;

    page_handle.page_id = page_id;
    page_handle.page_data_ptr = page_buf_ptr + sizeof(PageHdr);

    return true;
}

bool PageFileHandle::markDirty(int page_id)
{
    // file must be open
    if (!file_open)
    {
        return false;
    }

    // check if invalid
    if (page_id > hdr.pages_num || page_id < 0)
    {
        return false;
    }
    
    // let buffer manager to mark page dirty
    return buffer_mgr->markDirty(sys_fd, page_id);
}

bool PageFileHandle::unpinPage(int page_id)
{
    // file must be open
    if (!file_open)
    {
        return false;
    }

    // check if invalid
    if (page_id > hdr.pages_num || page_id < 0)
    {
        return false;
    }
    
    // let buffer manager to mark page dirty
    return buffer_mgr->unpinPage(sys_fd, page_id);
}

bool PageFileHandle::flushPages() 
{
    // file must be open
    if (!file_open)
    {
        return false;
    }

    if (hdr_changed)
    {
        if (lseek(sys_fd, 0, SEEK_SET) < 0)
        {
            return false;
        }

        int bytes_num = write(sys_fd, (char*)&hdr, sizeof(FileHeader));
        if (bytes_num < 0 || bytes_num != sizeof(FileHeader))
        {
            return false;
        }
        
        hdr_changed = false;
    }

    return buffer_mgr->flushPages(sys_fd);
}

bool PageFileHandle::forcePages(int page_id)    
{
    // file must be open
    if (!file_open)
    {
        return false;
    }

    if (hdr_changed)
    {
        if (lseek(sys_fd, 0, SEEK_SET) < 0)
        {
            return false;
        }

        int bytes_num = write(sys_fd, (char*)&hdr, sizeof(FileHeader));
        if (bytes_num < 0 || bytes_num != sizeof(FileHeader))
        {
            return false;
        }
        
        hdr_changed = false;
    }

    return buffer_mgr->forcePages(sys_fd, page_id);
}
