/*
 * File: pagefile_mgr.h
 * Description: Page file Manager interface
 * Author:
 * E-mail:
 *
 */

#pragma once

class BufferMgr;

class PageFileMgr
{
    private:
        BufferMgr *buffer_mgr_ptr;      // page buffer manager
    public:
        PageFileMgr();
        ~PageFileMgr();
        void createFile(const char *filename);
        void destoryFile(const char *filename);
        void openFile(const char *fileName, MDB_File &mdb_file);
        void closeFile(MDB_File &mdb_file);

        void clearBuffer();
        void printBuffer();
        void resizeBuffer();

        void getBlockSize(int &length) const;
        void allocateBlock(char *&buffer);
        void disposeBlock(char *&buffer);
};
