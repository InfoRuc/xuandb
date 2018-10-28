/*
 * File: buffer_page.h
 * Description: MiniDB buffer page class interface
 * Author:
 * E-mail:
 *
 */
#pragma once

class MDB_BufferPage
{
    private:
        char *bp_data_ptr;
        int next;
        int prev;
        bool if_dirty;
        int pin_count;
        int pg_id;
        int sys_fd;
    public:
        void getData(char *&bpd_ptr);
        void getPrevBP(int &next);
        void getNextBP(int &next);
        void getPinCount(int &pcount);
        void getPageID(int &pid);
        void getSysFD(int &sfd);
};

