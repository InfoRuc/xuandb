/*
 * File: storage_mgr_test.cpp
 * Description: test file for sm
 * Author: 
 * E-mail:
 *
 */
#include <iostream>
#include "common.h"
#include "sm/page_hashtable.h"
#include "sm/buffer_mgr.h"

using std::cout;
using std::endl;

void Page_HashTable_TEST(int buf_size = 40)
{
    PageHashTable pht; // using default size(20)
    BufPage *buf_table;
    int fd = 6;
    int slot = 0;

    buf_table = new BufPage[buf_size];
    // create 'buf_size' pages in buf_table to test and inset them into hashtable
    // TEST for insert
    for (int i = 0; i < buf_size; i++)
    {
        buf_table[i].data_ptr = new char[PAGE_WHOLE_SIZE];
        buf_table[i].if_dirty = false;
        buf_table[i].prev = buf_table[i].next = -1;
        buf_table[i].page_id = i;
        buf_table[i].fd = fd;
        if (!pht.insert(fd, buf_table[i].page_id, i))
        {
            cout << "HashTable insert failed in test " << i << endl;
            return;
        }
    }
    cout << "Test for hashtable *insert* success!" << endl;
     
    // TEST for search
    for (int i = 0; i < buf_size; i++)
    {
        int slot;
        pht.search(fd, buf_table[i].page_id, slot);
        if (buf_table[slot].page_id != i || slot != i)
        {
            cout << "HashTable search failed in test " << i << endl;
            return;
        }
    }
    cout << "Test for hashtable *insert* success!" << endl;
    
    // TEST for remove
    for (int i = 0; i < buf_size; i++)
    {
        int slot;
        pht.remove(fd, buf_table[i].page_id);
        if (pht.search(fd, buf_table[i].page_id, slot))
        {
            cout << "HashTable remove failed in test " << i << endl;
            return;
        }
    }
    cout << "Test for hashtable *remove* success!" << endl;

    cout << "**************Test for hashtable success!**************" << endl;
}

int main(int argc, char *argv[])
{
    cout << "**************Start test for page hashtable.**************" << endl;
    Page_HashTable_TEST();
    return 0;
}
