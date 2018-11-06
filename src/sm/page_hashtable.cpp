/*
 * File: page_hashtable.cpp
 * Description: buffer manager implemention
 * Author: Jiaqing Liu
 * E-mail: ljq929593357@163.com
 *
 */

#include <iostream>
#include "sm/page_hashtable.h"

using std::cout;
using std::endl;

//////////////////////////////////////////////////////////////
// Function: PageHashTable constructor
// Description: init PageHashTable
// Author: Jiaqing Liu
// E-mail: ljq929593357@163.com
//////////////////////////////////////////////////////////////
PageHashTable::PageHashTable(int ts)
{
    table_size = ts;
    // allocate memory for page hash table
    hash_table = new PageHashNode*[table_size];
    // init hash table with NULL
    for (int i = 0; i < table_size; i++)
        hash_table[i] = NULL;
}

//////////////////////////////////////////////////////////////
// Function: PageHashTable deconstructor
// Description: free PageHashTable
// Author: Jiaqing Liu
// E-mail: ljq929593357@163.com
//////////////////////////////////////////////////////////////
PageHashTable::~PageHashTable()
{
    // free hashtable
    for (int i = 0; i < table_size; i++)
    {
        PageHashNode *cur = hash_table[i];
        while(cur != NULL)
        {
            PageHashNode *next = cur->next;
            delete cur;
            cur = next;
        }
    }
}

//////////////////////////////////////////////////////////////
// Function: search
// Description: search a hash node by fd and page id
// Author: Jiaqing Liu
// E-mail: ljq929593357@163.com
//////////////////////////////////////////////////////////////
bool PageHashTable::search(int fd, int page_id, int &slot)
{
    // hash search by fd and page id
    int bucket = hash(fd, page_id);

    // has no hash bucket
    if (bucket < 0)
    {
        cout << "Error: search failed because page has no hash bucket!" << endl;
        return false;
    }

    // find the right page in the bucket
    for (PageHashNode *cur = hash_table[bucket]; cur != NULL; cur = cur->next)
    {
        if (cur->fd == fd && cur->page_id == page_id)
        {
#ifdef DEBUG
    cout << "Debug:Page is found in Hash Table." << endl;
#endif
            slot = cur->slot;  // get the slot of fd and page id
            return true;
        }
    }
    cout << "Error: search failed because page is not in bucket!" << endl;
    return false;
}

//////////////////////////////////////////////////////////////
// Function: insert
// Description: insert page into hash table
// Author: Jiaqing Liu
// E-mail: ljq929593357@163.com
//////////////////////////////////////////////////////////////
bool PageHashTable::insert(int fd, int page_id, int slot)
{
    // hash search by fd and page id
    int bucket = hash(fd, page_id);

    // check if page node exists
    for (PageHashNode *cur = hash_table[bucket]; cur != NULL; cur = cur->next)
    {
        if (cur->fd == fd && cur->page_id == page_id)
        {
            slot = cur->slot;
            cout << "Error: insert failed because page has existed!" << endl;
            return false;
        }
    }

    // new page hash node for inserted page
    PageHashNode *node = new PageHashNode(fd, page_id, slot, hash_table[bucket]);

    // update the hash table pointer
    if (hash_table[bucket] != NULL)
        hash_table[bucket]->prev = node;

    hash_table[bucket] = node;

    return true;
}

//////////////////////////////////////////////////////////////
// Function: remove
// Description: remove page from hash table
// Author: Jiaqing Liu
// E-mail: ljq929593357@163.com
//////////////////////////////////////////////////////////////
bool PageHashTable::remove(int fd, int page_id)
{
    // hash search by fd and page id
    int bucket = hash(fd, page_id);

    PageHashNode *node;

    // check if node is in the bucket
    for (node = hash_table[bucket]; node != NULL; node = node->next)
    {
        if (node->fd == fd && node->page_id == page_id)
        {
            break;
        }
    }

    // page not found
    if (node == NULL)
    {
        cout << "Error: remove failed because of page not found!" << endl;
        return false;
    }
    
    // remove this page node
    if(node == hash_table[bucket])
        hash_table[bucket] = node->next;
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;
    delete node;

    return true;
}

//////////////////////////////////////////////////////////////
// Function: hash
// Description: hash function by fd and page id
// Author: Jiaqing Liu
// E-mail: ljq929593357@163.com
//////////////////////////////////////////////////////////////
int PageHashTable::hash(int fd, int page_id) const
{
    // return hash function mapping result
    return ((fd + page_id) % table_size);
}

