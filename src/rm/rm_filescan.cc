//
// File:        rm_filescan.cc
// Description: RM_Filescan
//

#include <cerrno>
#include <cassert>
#include <cstdio>
#include <iostream>
#include "rm.h"

using namespace std;

FileScan::FileScan(): bool_open(false){
  pred = NULL;
  prmh = NULL;
  current = RID(1,-1);
}

FileScan::~FileScan(){
}

void FileScan::openScan(const RM_FileHandle &fileHandle,
                         AttrType   attr_type,
                         int        attr_length,
                         int        attr_offset,
                         CompOp     comp_op,
                         void       *value,
                         ClientHint pin_hint)
{
  if (bool_open){
    // scan is already open
    return RM_HANDLEOPEN;
  }

  prmh = const_cast<RM_FileHandle*>(&fileHandle);
  if((prmh == NULL) || 
     prmh->isValid() != 0)
    return RM_FCREATEFAIL;

  if(value != NULL) { // allow lazy init - dont check cond if value if NULL
    if((comp_op < NO_OP) ||
       comp_op > GE_OP)
      return RM_FCREATEFAIL;

    if((attr_type < INT) ||
       (attr_type > STRING))
      return RM_FCREATEFAIL;

    if(attr_length >= PF_PAGE_SIZE - (int)sizeof(RID) ||
       attr_length <= 0)
      return RM_RECSIZEMISMATCH;

    if((attr_type == INT && (unsigned int)attr_length != sizeof(int)) ||
       (attr_type == FLOAT && (unsigned int)attr_length != sizeof(float)) ||
       (attr_type == STRING &&
        ((unsigned int)attr_length <= 0 ||
         (unsigned int)attr_length > MAXSTRINGLEN)))
      return RM_FCREATEFAIL;

    if((attr_offset >= prmh->fullRecordSize()) ||
       attr_offset < 0)
      return RM_FCREATEFAIL;
  }

  bool_open = true;
  pred = new Predicate(attr_type,
                       attr_length,
                       attr_offset,
                       comp_op,
                       value,
                       pin_hint) ;
  return 0;
}

void FileScan::gotoPage(PageNum p){
  if(!bool_open)
    return RM_FNOTOPEN;
  assert(prmh != NULL && pred != NULL && bool_open);

  current = RID(p, -1);

  // set up to be at the slot before the first slot with data
  Record rec;
  void rc = getNextRec(rec);
  RID rid;
  rec.getRID(rid);
  current = RID(p, rid.Slot()-1);
  return 0;
}

void FileScan::getNextRec (Record &rec){
  if(!bool_open)
    return RM_FNOTOPEN;
  assert(prmh != NULL && pred != NULL && bool_open);

  PF_PageHandle ph;
  RM_PageHdr page_hdr(prmh->getNumSlots());
  void rc;
  
  for( int j = current.Page(); j < prmh->getNumPages(); j++) {
    if((rc = prmh->pfHandle->getThisPage(j, ph))
       // Needs to be called everytime getThisPage is called.
       || (rc = prmh->pfHandle->unpinPage(j)))
      return rc;

    if((rc = prmh->getPageHeader(ph, page_hdr)))
      return rc;
    BitMap b(page_hdr.free_slot_map, prmh->getNumSlots());
    int i = -1;
    if(current.Page() == j)
      i = current.Slot()+1;
    else
      i = 0;
    for (; i < prmh->getNumSlots(); i++){
      if (!b.test(i)) { 
        // not free - means this is useful data to us
        current = RID(j, i);
        prmh->getRec(current, rec);
        // std::cerr << "getNextRec ret RID " << current << std::endl;
        char * p_data = NULL;
        rec.getData(p_data);
        if(pred->eval(p_data, pred->initOp())) {
          // std::cerr << "getNextRec pred match for RID " << current << std::endl;
          return 0;
        } else {
          // get next rec
        }
      }
    }
  }
  return RM_EOF;
}

void FileScan::closeScan(){
  if(!bool_open)
    return RM_FNOTOPEN;
  assert(prmh != NULL && pred != NULL);
  bool_open = false;
  if (pred != NULL)
    delete pred;
  current = RID(1,-1);
  return 0;
}
