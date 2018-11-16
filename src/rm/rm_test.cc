//
// File:        rm_testshell.cc
// Description: Test RM component
//

#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#include "redbase.h"
#include "pf.h"
#include "rm.h"

using namespace std;

// Defines
const char * FILENAME =  "testrel";       // test file name
#define STRLEN     29     // length of string in testrec
#define PROG_UNIT  50     // how frequently to give progress reports when adding lots of recs
#define FEW_RECS   20     // number of records added in

// Computes the offset of a field in a record (should be in <stddef.h>)
#ifndef offsetof
#       define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif

// Structure of the records we will be using for the tests
struct TestRec {
    char  str[STRLEN];
    int   num;
    float r;
};

// Global PF_Manager and RM_Manager variables
PF_Manager pfm;
RM_Manager rmm(pfm);

// Function declarations
void Test1(void);
void Test2(void);

//void printErrorAll(void rc);
void lsFile(char *file_name);
void printRecord(TestRec &recBuf);
void addRecs(RM_FileHandle &fh, int numRecs);

void verifyFile(RM_FileHandle &fh, int numRecs);
void printFile(FileScan &fh);

void createFile(const char *file_name, int record_size);
void destroyFile(const char *file_name);
void openFile(const char *file_name, RM_FileHandle &fh);
void closeFile(const char *file_name, RM_FileHandle &fh);
void insertRec(RM_FileHandle &fh, char *record, RID &rid);
void updateRec(RM_FileHandle &fh, Record &rec);
void deleteRec(RM_FileHandle &fh, RID &rid);

void getNextRecScan(FileScan &fs, Record &rec);

// Array of pointers to the test functions
#define NUM_TESTS 2               // number of tests
int (*tests[])() = {              // void doesn't work on some compilers
    Test1,
    Test2
};

// main
int main(int argc, char *argv[]){
    void   rc;
    char *prog_name = argv[0];   // since we will be changing argv
    int  test_num;

    // Write out initial starting message
    cerr.flush();
    cout.flush();
    cout << "Starting RM component test.\n";
    cout.flush();

    // Delete files from last time
    unlink(FILENAME);

    // If no argument given, do all tests
    if (argc == 1) {
        for (test_num = 0; test_num < NUM_TESTS; test_num++)
            if ((rc = (tests[test_num])())) {

                // Print the error and exit
                // printErrorAll(rc);
                return (1);
            }
    }
    else {
        // Otherwise, perform specific tests
        while (*++argv != NULL) {
            // Make sure it's a number
            if (sscanf(*argv, "%d", &test_num) != 1) {
                cerr << prog_name << ": " << *argv << " is not a number\n";
                continue;
            }
            // Make sure it's in range
            if (test_num < 1 || test_num > NUM_TESTS) {
                cerr << "Valid test numbers are between 1 and " << NUM_TESTS << "\n";
                continue;
            }
            // Perform the test
            if ((rc = (tests[test_num - 1])())) {
                // Print the error and exit
                // printErrorAll(rc);
                return (1);
            }
        }
    }

    // Write ending message and exit
    cout << "Ending RM component test.\n\n";
    return 0;
}

// lsFile
// Desc: list the filename's directory entry
void lsFile(const char *file_name){
    char command[80];
    sprintf(command, "ls -l %s", file_name);
    printf("doing \"%s\"\n", command);
    system(command);
}

// printRecord
// Desc: Print the TestRec record components
void printRecord(TestRec &recBuf){
    printf("[%s, %d, %f]\n", recBuf.str, recBuf.num, recBuf.r);
}

// addRecs
// Desc: Add a number of records to the file
//
void addRecs(RM_FileHandle &fh, int numRecs){
    void      rc;
    int     i;
    TestRec recBuf;
    RID     rid;
    PageNum page_num;
    SlotNum slotNum;

    // We set all of the TestRec to be 0 initially.  This heads off
    // warnings that Purify will give regarding UMR since sizeof(TestRec)
    // is 40, whereas actual size is 37.
    memset((void *)&recBuf, 0, sizeof(recBuf));

    printf("\nadding %d records\n", numRecs);
    for (i = 0; i < numRecs; i++) {
        memset(recBuf.str, ' ', STRLEN);
        sprintf(recBuf.str, "a%d", i);
        recBuf.num = i;
        recBuf.r = (float)i;
        if ((rc = insertRec(fh, (char *)&recBuf, rid)) ||
            (rc = rid.getPageNum(page_num)) ||
            (rc = rid.getSlotNum(slotNum)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0){
            printf("%d  ", i + 1);
            fflush(stdout);
        }
    }
    if (i % PROG_UNIT != 0)
        printf("%d\n", i);
    else
        putchar('\n');

    // Return ok
    return 0;
}

// verifyFile
// Desc: verify that a file has records as added by addRecs
void verifyFile(RM_FileHandle &fh, int numRecs){
    void        rc;
    int       n;
    TestRec   *pRecBuf;
    RID       rid;
    char      string_buf[STRLEN];
    char      *found;
    Record rec;

    found = new char[numRecs];
    memset(found, 0, numRecs);

    printf("\nverifying file contents\n");

    FileScan fs;
    if ((rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                        NO_OP, NULL, NO_HINT)))
    // int val = 10;
    // if ((rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
    //                     LT_OP, (void*)&val, NO_HINT)))
    // const char * grr = "a15";
    // if ((rc=fs.openScan(fh,STRING,3,offsetof(TestRec, str),
    //                     GE_OP, (void*)grr, NO_HINT)))
        return (rc);

    // For each record in the file
    for (rc = getNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = getNextRecScan(fs, rec), n++) {

        // Make sure the record is correct
        if ((rc = rec.getData((char *&)pRecBuf)) ||
            (rc = rec.getRID(rid)))
            goto err;

        memset(string_buf,' ', STRLEN);
        sprintf(string_buf, "a%d", pRecBuf->num);

        if (pRecBuf->num < 0 || pRecBuf->num >= numRecs ||
            strcmp(pRecBuf->str, string_buf) ||
            pRecBuf->r != (float)pRecBuf->num) {
            printf("verifyFile: invalid record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            printf("Nandu expected RID[%d,%d] = [%s, %d, %f]\n",
                   rid.Page(), rid.Slot(), string_buf, pRecBuf->num, pRecBuf->r);
            exit(1);
        }

        if (found[pRecBuf->num]){
            printf("verifyFile: duplicate record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            exit(1);
        }
        printf("Nandu found RID[%d,%d] = [%s, %d, %f]\n",
               rid.Page(), rid.Slot(), string_buf, pRecBuf->num, pRecBuf->r);
        found[pRecBuf->num] = 1;
    }

    if (rc != RM_EOF)
        goto err;

    if ((rc=fs.closeScan()))
        goto err;

    // make sure we had the right number of records in the file
    if (n != numRecs) {
        delete[] found;
        printf("%d records in file (supposed to be %d)\n", n, numRecs);
        exit(1);
    }

    // Return ok
    rc = 0;

err:
    fs.closeScan();
    delete[] found;
    return (rc);
}

// printFile
// Desc: Print the contents of the file
void printFile(FileScan &fs){
    void        rc;
    int       n;
    TestRec   *pRecBuf;
    RID       rid;
    Record rec;

    printf("\nprinting file contents\n");

    // for each record in the file
    for (rc = getNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = getNextRecScan(fs, rec), n++) {

        // Get the record data and record id
        if ((rc = rec.getData((char *&)pRecBuf)) ||
            (rc = rec.getRID(rid)))
            return (rc);

        // Print the record contents
        printRecord(*pRecBuf);
    }

    if (rc != RM_EOF)
        return (rc);

    printf("%d records found\n", n);

    // Return ok
    return (0);
}

// createFile
// Desc: call RM_Manager::createFile
void createFile(const char *file_name, int record_size){
    printf("\ncreating %s\n", file_name);
    return (rmm.createFile(file_name, record_size));
}

// destroyFile
// Desc: call RM_Manager::destroyFile
void destroyFile(const char *file_name){
    printf("\ndestroying %s\n", file_name);
    return (rmm.destroyFile(file_name));
}

// openFile
// Desc: call RM_Manager::openFile

void openFile(const char *file_name, RM_FileHandle &fh){
    printf("\nopening %s\n", file_name);
    return (rmm.openFile(file_name, fh));
}

// closeFile
// Desc: call RM_Manager::closeFile
void closeFile(const char *file_name, RM_FileHandle &fh){
    if (file_name != NULL)
        printf("\nClosing %s\n", file_name);
    return (rmm.closeFile(fh));
}

// insertRec
// Desc: call RM_FileHandle::insertRec
void insertRec(RM_FileHandle &fh, char *record, RID &rid){
    return (fh.insertRec(record, rid));
}

// deleteRec
// Desc: call RM_FileHandle::deleteRec
void deleteRec(RM_FileHandle &fh, RID &rid){
    return (fh.deleteRec(rid));
}

// updateRec
// Desc: call RM_FileHandle::updateRec
void updateRec(RM_FileHandle &fh, Record &rec){
    return (fh.updateRec(rec));
}

// getNextRecScan
// Desc: call FileScan::getNextRec
void getNextRecScan(FileScan &fs, Record &rec){
    return (fs.getNextRec(rec));
}

// Test1 tests simple creation, opening, closing, and deletion of files
void Test1(void){
    void rc;
    RM_FileHandle fh;

    printf("test1 starting ****************\n");

    if (
        (rc = createFile(FILENAME, sizeof(TestRec)))
        || (rc = openFile(FILENAME, fh))
        || (rc = closeFile(FILENAME, fh))
      )
        return (rc);

    lsFile(FILENAME);

    if ((rc = destroyFile(FILENAME)))
        return (rc);

    printf("\ntest1 done ********************\n");
    return 0;
}

// Test2 tests adding a few records to a file.
void Test2(void){
    void rc;
    RM_FileHandle fh;

    printf("test2 starting ****************\n");

    if ((rc = createFile(FILENAME, sizeof(TestRec))) ||
        (rc = openFile(FILENAME, fh)) ||
        (rc = addRecs(fh, FEW_RECS)) ||
        (rc = verifyFile(fh, FEW_RECS)) ||
        (rc = closeFile(FILENAME, fh)))
        return rc;

    lsFile(FILENAME);
    // printFile(fh);

    if ((rc = destroyFile(FILENAME)))
        return rc;

    printf("\ntest2 done ********************\n");
    return 0;
}
