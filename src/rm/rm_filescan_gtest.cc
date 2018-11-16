#include "rm_rid.h"
#include "rm.h"
#include "gtest/gtest.h"

#define STRLEN 29
struct TestRec {
    char  str[STRLEN];
    int   num;
    float r;
};

class FileScanTest : public ::testing::Test {
protected:
	FileScanTest(): rmm(pfm){
	}
	virtual void setUp(){
		void rc;
		system("rm -f gtestfile");
		if(
			(rc = rmm.createFile("gtestfile", sizeof(TestRec)))
			 || (rc =	rmm.openFile("gtestfile", fh))
			)
			printError(rc);
	}

	virtual void TearDown(){
		rmm.destroyFile("gtestfile");
	}
  // Declares the variables your tests want to use.
	PF_Manager pfm;
	RM_Manager rmm;
	RM_FileHandle fh;
};

// Setup will call both constructor and Open()
TEST_F(FileScanTest, Update) {
	FileScan fs;
	void rc;

  TestRec t;
  RID r;
	std::vector<RID> vec;
	int count = 400;
	for( int i = 0; i < count; i++){
		t.num = i;
		fh.insertRec((char*) &t, r);
		vec.push_back(r);
	}
	ASSERT_EQ(fh.getNumPages(), 5);

	// check within same open
	for( int i = 0; i < count; i++){
		Record rec;
		fh.getRec(vec[i], rec);
		TestRec * pBuf;
		rec.getData((char*&)pBuf);
		// std::cerr << vec[i] << "pBuf->num " << pBuf->i << " i " << i << std::endl;
		ASSERT_EQ(pBuf->num, i);
	}

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  int numRecs = 0;
  while(1) {
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count);

// update
  for( int i = 0; i < 10; i++){
    int j = random() % count;
    Record rec;
		fh.getRec(vec[j], rec);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    vec[pBuf->num] = RID(-1,-1);
    pBuf->num = vec.size();
    fh.updateRec(rec);
    RID rid;
    rec.getRID(rid);
    vec.push_back(rid);
  }

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  numRecs = 0;
  while(1) {
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    // cerr << pBuf->num << endl;
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count);
}


TEST_F(FileScanTest, UpdateAll) {
	FileScan fs;
	void rc;

  TestRec t;
  RID r;
	std::vector<RID> vec;
	int count = 4000;
	for( int i = 0; i < count; i++)
	{
		t.num = i;
		fh.insertRec((char*) &t, r);
		vec.push_back(r);
	}
	//ASSERT_EQ(fh.getNumPages(), 5);

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  int numRecs = 0;
  while(1) {
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count);

// update all
  for( int i = 0; i < count; i++){
    Record rec;
		fh.getRec(vec[i], rec);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    vec[pBuf->num] = RID(-1,-1);
    pBuf->num = vec.size();
    fh.updateRec(rec);
    RID rid;
    rec.getRID(rid);
    vec.push_back(rid);
  }

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  numRecs = 0;
  while(1) {
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(pBuf->num, count + numRecs);
    EXPECT_EQ(vec[pBuf->num], (rid));
    // cerr << pBuf->num << "\t" << rid << endl;
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count);

  int val = 7999;
  void * pval = &val;
  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  EQ_OP, pval, NO_HINT));
  ASSERT_EQ(rc, 0);

  numRecs = 0;
  while(1) {
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    // cerr << pBuf->num << "\t" << rid << endl;
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, 1);

}

TEST_F(FileScanTest, Delete) {
	FileScan fs;
	void rc;

  TestRec t;
  RID r;
	std::vector<RID> vec;
	int count = 400;
	for( int i = 0; i < count; i++){
		t.num = i;
		fh.insertRec((char*) &t, r);
		vec.push_back(r);
	}

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  int numRecs = 0;
  while(1) {
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count);

// Delete
	for( int i = 0; i < 10; i++){
    int j = random() % count;
    fh.deleteRec(vec[j]);
    vec[j] = RID(-1,-1);
  }

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  numRecs = 0;
  while(1){
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    numRecs++;
  }  

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count-10);
}

TEST_F(FileScanTest, DeleteDuring){
	FileScan fs;
	void rc;

  TestRec t;
  RID r;
	std::vector<RID> vec;
	int count = 400;
	for( int i = 0; i < count; i++){
		t.num = i;
		fh.insertRec((char*) &t, r);
		vec.push_back(r);
	}

  (rc=fs.openScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                  NO_OP, NULL, NO_HINT));
  ASSERT_EQ(rc, 0);

  int numRecs = 0;
  while(numRecs < count/2){
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    // cerr << pBuf->num << "\t" << rid << endl;
    numRecs++;
  }

// Delete from second half
	for( int i = 0; i < 10; i++){
    int j = random() % count/2;
    j += count/2;
    // cerr << "Deleting " << "\t" << j << endl;
    rc = fh.deleteRec(vec[j]);
    EXPECT_EQ(0, rc);

    Record rec;
    rc = fh.getRec(vec[j], rec);
    EXPECT_NE(0, rc); // rec should no longer be returned.

    vec[j] = RID(-1,-1);
  }

  while(1){
    Record rec;
    rc = fs.getNextRec(rec);
    if(rc == RM_EOF)
      break;
    EXPECT_EQ(rc, 0);
    TestRec * pBuf;
		rec.getData((char*&)pBuf);
    RID rid;
    rec.getRID(rid);
    EXPECT_EQ(vec[pBuf->num], (rid));
    // cerr << pBuf->num << "\t" << rid << endl;

    numRecs++;
  }

  (rc=fs.closeScan());
  ASSERT_EQ(rc, 0);
  ASSERT_EQ(numRecs, count-10);

}
