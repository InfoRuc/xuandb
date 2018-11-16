/*
 * File: storage_mgr_test.cpp
 * Description: test file for sm
 * Author: 
 * E-mail:
 *
 */
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include "common.h"
#include "sm/buffer_mgr.h"
#include "sm/storage_mgr.h"
#include "sm/page_handle.h"

using std::cout;
using std::endl;

void Buffer_Mgr_Test(int buf_size = 40, int bench_size = 45)
{
    // Get Page Test
    BufferMgr bm(buf_size);
    char file_name[32] = "buf_test_data.mdb";
    int sys_fd = open(file_name, O_CREAT | O_RDWR, 0600);
    if (sys_fd < 0)
    {
        cout << "[BufferMgr Test Error]: Open file failed!" << endl;
        return;
    }

    // write pages manually
    int page_size = PAGE_WHOLE_SIZE;
    char *bench_data = new char[page_size];
    memset(bench_data, 1, page_size);
    
    int file_header_size = FILE_HEADER_SIZE;
    char file_header[file_header_size];

    cout << "[BufferMgr Test Info]: Generating benchmark data..." << endl;
    memset(file_header, 0, file_header_size);
    FileHeader* hdr_buf = (FileHeader*)file_header;
    hdr_buf->first_free = -1;
    hdr_buf->pages_num = bench_size;

    // generate benchmark data
    int offset = file_header_size;
    int bytes_num = write(sys_fd, hdr_buf, file_header_size);

    if (bytes_num == -1)
    {
        perror("[BufferMgr Test Error]: Read page error");
        return;
    }

    if (bytes_num == 0)
    {
        cout << "[BufferMgr Test Error]: Read arrived to EOF!" << endl;
        return;
    }

    if (bytes_num < file_header_size)
    {
        cout << "[BufferMgr Error]: Read incomplete page only " << bytes_num << " bytes!"<< endl;
        return;
    }

    if (lseek(sys_fd, offset, SEEK_SET) < 0)
    {
        cout << "[BufferMgr Test Error]: Seek page failed." << endl;
        return;
    }

    offset += file_header_size;
    for (int i = 0; i < bench_size; i++)
    {
        ((PageHdr*)bench_data)[0] = i + 1;
        bytes_num = write(sys_fd, bench_data, page_size);
        if (bytes_num == -1)
        {
             perror("[BufferMgr Test Error]: Read page error");
            return;
        }

        if (bytes_num == 0)
        {
            cout << "[BufferMgr Test Error]: Read arrived to EOF!" << endl;
            return;
        }

        if (bytes_num < page_size)
        {
            cout << "[BufferMgr Error]: Read incomplete page only " << bytes_num << " bytes!"<< endl;
            return;
        }

        if (lseek(sys_fd, offset, SEEK_SET) < 0)
        {
            cout << "[BufferMgr Test Error]: Seek page failed." << endl;
            return;
        }
        offset += page_size;
    }
    cout << "[BufferMgr Test Info]: Benchmark data '"<< file_name << "' generation succeeded." << endl;
    
    char *pg_data;
    cout << "[BufferMgr Test Info]: "<< "Get 40 pages into buffer pool." << endl;
    for (int i = 0; i < buf_size; i++)
    {
        bool page_gotten = bm.getPage(sys_fd, i, pg_data);
        if (!page_gotten)
        {
            cout << "[BufferMgr Test Error]: Get #" << i << " page failed!" << endl;
            return;
        }
        //sleep(1);
    }
    cout << "[BufferMgr Test Info]: "<< "Test loading page into buffer pool when buffer full." << endl;
    bool page_gotten = bm.getPage(sys_fd, buf_size, pg_data);
    if (page_gotten)
    {
        cout << "[BufferMgr Test Error]: " << "Buffer overflow test failed!" << endl;
        return;
    }
    
    cout << "[BufferMgr Test Info]: "<< "Test writing page into disk when page is dirty." << endl;
    int test;
    test = 9;
    cout << "[BufferMgr Test Info]: "<< "Mark #" << test << " page dirty." << endl;
    if (!bm.markDirty(sys_fd, test))
    {
        cout << "[BufferMgr Test Error]: " << "#" << test << " page mark failed!" << endl;
        return;
    }
    
    // test for forcePage function
    bm.forcePage(sys_fd, test);
    for (int i = 0; i < buf_size; i++)
    {
        if (i != test)
        {
            if (!bm.markDirty(sys_fd, i))
            {
                cout << "[BufferMgr Test Error]: " << "#" << i << " page mark failed!" << endl;
                return;
            }
            if (!bm.unpinPage(sys_fd, i))
            {
                cout << "[BufferMgr Test Error]: " << "#" << i << " page unpin failed!" << endl;
                return;
            }
        }
    }
    if (!bm.flushPages(sys_fd))
    {
        cout << "[BufferMgr Test Error]: " <<  "Flush pages failed!" << endl;
        return;
    }
    close(sys_fd);
}

int main(int argc, char *argv[])
{
    cout << "************** Start test for buffer manager **************" << endl;
    Buffer_Mgr_Test();
    return 0;
}
