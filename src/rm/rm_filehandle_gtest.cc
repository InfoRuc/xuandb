#include "rm_rid.h"
#include "rm.h"
#include "gtest/gtest.h"

#define STRLEN 29
struct TestRec {
    char  str[STRLEN];
    int   num;
    float r;
};

class RM_FileHandleTest : public ::testing::Test {
protected:
	RM_FileHandleTest(): rmm(pfm)
		{}
	virtual void setUp(){
		void rc;
		system("rm -f gtestfile");
		if(
			(rc = rmm.createFile("gtestfile", sizeof(TestRec)))
			 || (rc = rmm.openFile("gtestfile", fh))
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

TEST_F(RM_FileHandleTest, FullSize) {
	ASSERT_EQ(40UL ,sizeof(TestRec));
	ASSERT_EQ(40 ,fh.fullRecordSize());
	ASSERT_EQ(101, fh.getNumSlots());
}

TEST_F(RM_FileHandleTest, ZeroRec) {
	RM_FileHandle sfh;
	void rc;
 	system("rm -f gtestfilesmall");
  (rc = rmm.createFile("gtestfilesmall", 0));
  ASSERT_EQ(rc, RM_BADRECSIZE);

	(rc =	rmm.openFile("gtestfilesmall", sfh));
  ASSERT_LT(rc, 0);

	rc = rmm.closeFile(sfh);
  ASSERT_LT(rc, 0);

	rc = rmm.destroyFile("gtestfilesmall");
  ASSERT_LT(rc, 0);
}

TEST_F(RM_FileHandleTest, SmallRec) {
	struct SmallRec {
		int i;
	};
	RM_FileHandle sfh;
	void rc;
 	system("rm -f gtestfilesmall");
	if(
		(rc = rmm.createFile("gtestfilesmall", sizeof(SmallRec)))
		|| (rc =	rmm.openFile("gtestfilesmall", sfh))
		)
		printError(rc);

	ASSERT_EQ(4UL, sizeof(SmallRec));
	ASSERT_EQ(4 ,sfh.fullRecordSize());
	ASSERT_EQ(988, sfh.getNumSlots());

	rmm.closeFile(sfh);
	rmm.destroyFile("gtestfilesmall");
}


TEST_F(RM_FileHandleTest, Persist) {
	void rc;
	RM_FileHandle fh2;

	system("rm -f gtestfile2");
	if(
		(rc = rmm.createFile("gtestfile2", sizeof(TestRec))) < 0
		)
	printError(rc);
	ASSERT_EQ(RM_FNOTOPEN, rmm.closeFile(fh2));

	if ((rc =	rmm.openFile("gtestfile2", fh2)) < 0
		|| (rc =	rmm.closeFile(fh2)) < 0
		|| (rc =	rmm.openFile("gtestfile2", fh2)) < 0
		)
		printError(rc);

	ASSERT_EQ(fh2.fullRecordSize(), 40);
	ASSERT_EQ(fh2.getNumPages(), 1);
	
	rmm.closeFile(fh2);
	rmm.destroyFile("gtestfile2");
}


TEST_F(RM_FileHandleTest, WrongDelGet) {
	TestRec t;
	RID rid;
  void rc;
  Record rec;

	rc = fh.deleteRec(RID(100,100));
  ASSERT_NE(0, rc);

	rc = fh.insertRec((char*) &t, rid);
  ASSERT_EQ(0, rc);
  
	rc = fh.deleteRec(rid);
  ASSERT_EQ(0, rc); // first ok

	rc = fh.deleteRec(rid);
  ASSERT_NE(0, rc); // second should fail

  rec.set((char*)&t, sizeof(t), rid);
	rc = fh.updateRec(rec);
  ASSERT_NE(0, rc); // update should also fail

	rc = fh.getRec(rid, rec);
  ASSERT_NE(0, rc); // update should also fail

  rec.set((char*)&t, sizeof(t), RID(1000, 1000));
	rc = fh.updateRec(rec);
  ASSERT_NE(0, rc); // should fail

	rc = fh.insertRec((char*) &t, rid);
  ASSERT_EQ(0, rc);

  rec.set((char*)&t, sizeof(t), rid);
	rc = fh.updateRec(rec);
  ASSERT_EQ(0, rc);

	rc = fh.updateRec(rec);
  ASSERT_EQ(0, rc); // 2nd should work too
}

TEST_F(RM_FileHandleTest, FreeList) {
	TestRec t;
	RID p1rid;
	RID p2rid;
	fh.insertRec((char*) &t, p1rid);
	// std::cerr << "p1 RID was " << p1rid << std::endl;
  // std::cerr << "slots were " << fh.getNumSlots() << std::endl;
	for( int i = 0; i < fh.getNumSlots() + 5; i++){
		t.num = i;
		void rc = fh.insertRec((char*) &t, p2rid);
    ASSERT_EQ(rc, 0);
		// std::cerr << "p2 RID was " << p2rid << std::endl;
	}
	// p1 should be full
	ASSERT_EQ(fh.getNumPages(), 3);
	// std::cerr << "p2 RID was " << p2rid << std::endl;
	fh.deleteRec(p1rid);
	RID newr;
	fh.insertRec((char*) &t, newr);
	ASSERT_EQ(newr, p1rid);
	fh.insertRec((char*) &t, newr);
	fh.insertRec((char*) &t, newr);
}

TEST_F(RM_FileHandleTest, SmallRecIntegrity) {
	struct SmallRec {
		int i;
    int j;
    int k;
	};
	RM_FileHandle sfh;
	void rc;
 	system("rm -f gtestfilesmall");
	if(
		(rc = rmm.createFile("gtestfilesmall", sizeof(SmallRec)))
		|| (rc =	rmm.openFile("gtestfilesmall", sfh))
		)
		printError(rc);

	ASSERT_EQ(sfh.getNumPages(), 1);

	SmallRec t;
	RID r;
	std::vector<RID> vec;
	int count = 400;
	for( int i = 0; i < count; i++){
		t.i = i;
		sfh.insertRec((char*) &t, r);
		vec.push_back(r);
	}
	ASSERT_EQ(sfh.getNumPages(), 3);

	// check within same open
	for( int i = 0; i < count; i++){
		Record rec;
		sfh.getRec(vec[i], rec);
		SmallRec * pBuf;
		rec.getData((char*&)pBuf);
		// std::cerr << vec[i] << "pBuf->i " << pBuf->i << " i " << i << std::endl;
		ASSERT_EQ(pBuf->i, i);
	}

	// check with new open
	rmm.closeFile(sfh);
	RM_FileHandle sfh2;

	rc = rmm.openFile("gtestfilesmall", sfh2);
	ASSERT_EQ(sfh2.getNumPages(), 3);

	for( int i = 0; i < count; i++){
		Record rec;
		sfh2.getRec(vec[i], rec);
		SmallRec * pBuf;
		rec.getData((char*&)pBuf);
		// std::cerr << vec[i] << "pBuf->i " << pBuf->i << " i " << i << std::endl;
		ASSERT_EQ(pBuf->i, i);
	}
	rmm.closeFile(sfh2);
	rmm.destroyFile("gtestfilesmall");
}

TEST_F(RM_FileHandleTest, over40pages) {
  for( int page = 1; page < 60; page++) {
    RID rid;
    for ( int record = 0; record < fh.getNumSlots(); record++ ){
      TestRec t;
      void rc = fh.insertRec((char *)&t, rid);
      ASSERT_EQ(rc, 0);
    }
    ASSERT_EQ(rid, RID(page, fh.getNumSlots()-1));
  }
}
