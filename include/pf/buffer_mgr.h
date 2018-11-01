/*
 * File: buffer_mgr.h
 * Description: buffer manager interface
 * Author: pinyin of your name
 * E-mail:
 *
 */
#pragma once

//
// BufPageDesc - struct containing data about a page in the buffer
//
struct BufPageDesc
{
    char *data_ptr;     // page contents
    int next;       // next in the linked list of buffer pages
    int prev;       // prev in the linked list of buffer pages
    bool if_dirty;  // TRUE if page is dirty
    int pin_count;  // pin count
    int pg_id;      // page number for this page
    int sys_fd;     // OS file descriptor of this page
};

//
// BufferMgr
// manageing the page buffer
//
class BufferMgr
{

};
