//
// File:        rm_record.cc
// Description: RM_Record
//

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "rm.h"
#include "rm_rid.h"

using namespace std;

Record::Record()
  :record_size(-1), data(NULL), rid(-1,-1)
{
}

Record::~Record(){
	if (data != NULL) {
		delete [] data;
	}
}

// int Record::getRecordSize() const{
// 	return record_size;
// }

// Allows a resetting as long as size matches.
void Record::set(char *p_data, int size, RID rid_){
	if(record_size != -1 && (size != record_size))
		return RM_RECSIZEMISMATCH;
	record_size = size;
  this->rid = rid_;
	if (data == NULL)
		data = new char[record_size];
  memcpy(data, p_data, size);
	return 0;
}

void Record::getData(char *&p_data) const{
	if (data != NULL && record_size != -1){
		p_data = data;
		return 0;
	}
	else 
		return RM_NULLRECORD;
}

void Record::getRID(RID &rid) const{
	if (data != NULL && record_size != -1){
    rid = this->rid;
    return 0;
  }
	else 
		return RM_NULLRECORD;
}

ostream& operator <<(ostream & os, const RID& r){
  PageNum p;
  SlotNum s;
  r.getPageNum(p);
  r.getSlotNum(s);
  os << "[" << p << "," << s << "]";
  return os;
}
