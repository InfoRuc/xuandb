/*
 * File: storage_mgr_test.cpp
 * Description: test file for sm
 * Author: 
 * E-mail:
 *
 */
#include <iostream>
#include "sm/storage_mgr.h"
#include "common.h"

using std::cout;
using std::endl;

void Storage_Mgr_Test(int buf_size = 40)
{
    StorageMgr sm;
    char file_name[32] = "record_data.mdb";

    // storage manager init test
    if (!sm.initSM(file_name))
    {
        cout << "Storage Manager init failed." << endl;
        return;
    }
    cout << "Storage Manager init succeeded." << endl;

    // write 50 pages manually
    //sm.

}

int main(int argc, char *argv[])
{
    cout << "**************Start test for storage manager.**************" << endl;
    Storage_Mgr_Test();
    return 0;
}
