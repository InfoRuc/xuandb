//
// File:        rm_filehandle.cc
// Description: RM_Filehandle
//

#include <cerrno>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <cassert>
#include "rm.h"

using namespace std;

RM_FileHandle::RM_FileHandle() 
  :pfHandle(NULL), bool_file_open(false), bool_hdr_changed(false)
{
}

void RM_FileHandle::Open(PF_FileHandle* pfh, int size){
  if(bool_file_open || pfHandle != NULL) {
    return RM_HANDLEOPEN; 
  }
  if(pfh == NULL || size <= 0) {
    return RM_FCREATEFAIL;
  }
  bool_file_open = true;
  pfHandle = new PF_FileHandle;
  *pfHandle = *pfh ;

  PF_PageHandle ph;
  pfHandle->getThisPage(0, ph);
  // Needs to be called everytime getThisPage is called.
  pfHandle->unpinPage(0);

{ // testing
       char * p_data;
       ph.getData(p_data);
       RM_FileHdr hdr;
       memcpy(&hdr, p_data, sizeof(hdr));
       // std::cerr << "RM_FileHandle::Open inner hdr.num_of_pages" << hdr.num_of_pages << std::endl;
  }

  this->getFileHeader(ph); // write into hdr
  // std::cerr << "RM_FileHandle::Open hdr.num_of_pages" << hdr.num_of_pages << std::endl;

  
  bool_hdr_changed = true;
  void invalid = isValid(); if(invalid) return invalid;

  return 0;
}

void RM_FileHandle::getPF_FileHandle(PF_FileHandle &lvalue) const{
  void invalid = isValid(); if(invalid) return invalid;
  lvalue = *pfHandle;
  return 0;
}

void RM_FileHandle::getNextFreeSlot(PF_PageHandle & ph, PageNum& page_num, SlotNum& slotNum){
  void invalid = isValid(); if(invalid) return invalid;

  RM_PageHdr page_hdr(this->getNumSlots());
  void rc;
  
  if ((rc= this->getNextFreePage(page_num))
      || (rc = pfHandle->getThisPage(page_num, ph))
      // Needs to be called everytime getThisPage is called.
      || (rc = pfHandle->unpinPage(page_num))
      || (rc = this->getPageHeader(ph, page_hdr)))
      return rc;
  BitMap b(page_hdr.free_slot_map, this->getNumSlots());
  // std::cerr << "RM_FileHandle::getNextFreeSlot from page_hdr" << b << endl;
  for (int i = 0; i < this->getNumSlots(); i++)
  {
    if (b.test(i)) { 
      slotNum = i;
      return 0;
    }
  }
  // This page is full
  return -1; // unexpected error
}

void RM_FileHandle::getNextFreePage(PageNum& page_num){
  void invalid = isValid(); if(invalid) return invalid;
  PF_PageHandle ph;
  RM_PageHdr page_hdr(this->getNumSlots());
  PageNum p;

  if(hdr.first_free_page != RM_PAGE_LIST_END){
    // this last page on the free list might actually be full
    void rc;
    if ((rc = pfHandle->getThisPage(hdr.first_free_page, ph))
        || (rc = ph.getPageNum(p))
        || (rc = pfHandle->markDirty(p))
        // Needs to be called everytime getThisPage is called.
        || (rc = pfHandle->unpinPage(hdr.first_free_page))
        || (rc = this->getPageHeader(ph, page_hdr)))
      return rc;

  }
  if( //we need to allocate a new page because this is the firs time
    hdr.num_of_pages == 0 ||
    hdr.first_free_page == RM_PAGE_LIST_END ||
    // or due to a full page
    // (page_hdr.num_of_free_slots == 0 && page_hdr.next_free_page == RM_PAGE_FULLY_USED)
    (page_hdr.num_of_free_slots == 0)
    ) 
  {

    if(page_hdr.next_free_page == RM_PAGE_FULLY_USED){
      // std::cerr << "RM_FileHandle::getNextFreePage - Page Full!" << endl;
    }
    {
      char *p_data;

      void rc;
      if ((rc = pfHandle->AllocatePage(ph)) ||
          (rc = ph.getData(p_data)) ||
          (rc = ph.getPageNum(page_num)))
        return(rc);
      
      // Add page header
      RM_PageHdr phdr(this->getNumSlots());
      phdr.next_free_page = RM_PAGE_LIST_END;
      BitMap b(this->getNumSlots());
      b.set(); // Initially all slots are free
      b.toCharBuf(phdr.free_slot_map, b.numChars());
      // std::cerr << "RM_FileHandle::getNextFreePage new!!" << b << endl;
      phdr.toBuf(p_data);

      // the default behavior of the buffer pool is to pin pages
      // let us make sure that we unpin explicitly after setting
      // things up
      if (
      //(rc = pfHandle->markDirty(page_num)) ||
        (rc = pfHandle->unpinPage(page_num)))
        return rc;
    }

    { // testing
      PF_PageHandle ph;
      pfHandle->getThisPage(page_num, ph);
      // Needs to be called everytime getThisPage is called.
      pfHandle->unpinPage(page_num);

      RM_PageHdr page_hdr(this->getNumSlots());
      void rc;
      (rc = this->getPageHeader(ph, page_hdr));
      BitMap b(page_hdr.free_slot_map, this->getNumSlots());

      // std::cerr << "RM_FileHandle::getNextFreePage regen" << b << endl;
    }


    // add page to the free list
    hdr.first_free_page = page_num;
    hdr.num_of_pages++;
    assert(hdr.num_of_pages > 1); // page num 1 would be header page
    // std::cerr << "RM_FileHandle::getNextFreePage hdr.num_of_pages is "
    //           << hdr.num_of_pages
    //           << " method " << this->getNumPages()
    //           << endl;
    bool_hdr_changed = true;
    return 0; // page_num is set correctly
  }
  // return existing free page
  page_num = hdr.first_free_page;
  return 0;
}

void RM_FileHandle::getPageHeader(PF_PageHandle ph, RM_PageHdr& page_hdr) const{
  char * buf;
  void rc = ph.getData(buf);
  page_hdr.fromBuf(buf);
  return rc;
}

void RM_FileHandle::setPageHeader(PF_PageHandle ph, const RM_PageHdr& page_hdr){
  char * buf;
  void rc;
  if((rc = ph.getData(buf)))
    return rc;
  page_hdr.toBuf(buf);
  return 0;
}

// get header from the first page of a newly opened file
void RM_FileHandle::getFileHeader(PF_PageHandle ph){
  char * buf;
  void rc = ph.getData(buf);
  memcpy(&hdr, buf, sizeof(hdr));
  return rc;
}

// persist header into the first page of a file for later
void RM_FileHandle::setFileHeader(PF_PageHandle ph) const{
  char * buf;
  void rc = ph.getData(buf);
  memcpy(buf, &hdr, sizeof(hdr));
  return rc;
}

void RM_FileHandle::getSlotPointer(PF_PageHandle ph, SlotNum s, char *& p_data) const{
  void invalid = isValid(); if(invalid) return invalid;
  void rc = ph.getData(p_data);
  if (rc >= 0 ) {
    BitMap b(this->getNumSlots());
    p_data = p_data + (RM_PageHdr(this->getNumSlots()).size());
    p_data = p_data + s * this->fullRecordSize();
  }
  
  return rc;
}
 
int RM_FileHandle::getNumSlots() const{
  if(this->fullRecordSize() != 0) {
    int bytes_available = PF_PAGE_SIZE - sizeof(RM_PageHdr);
    int slots = floor(1.0 * bytes_available/ (this->fullRecordSize() + 1/8));
    int r = sizeof(RM_PageHdr) + BitMap(slots).numChars();
    
    while ((slots*this->fullRecordSize()) + r > PF_PAGE_SIZE) {
       slots--;
      r = sizeof(RM_PageHdr) + BitMap(slots).numChars();
      // std::cerr << "PF_PAGE_SIZE " << PF_PAGE_SIZE << std::endl;
      // std::cerr << " slots*frs " << slots*this->fullRecordSize()
      //           << " r "<< r 
      //           << " frs " << this->fullRecordSize()
      //           <<  std::endl;
    }
    
    // std::cerr << "Final  " << std::endl;
    // std::cerr << " slots*frs " << slots*this->fullRecordSize()
    //           << " r "<< r 
    //           << " frs " << this->fullRecordSize()
    //           <<  std::endl;
    return slots;
  }
  else {
    return RM_RECSIZEMISMATCH;
  }
}

RM_FileHandle::~RM_FileHandle(){
  if(pfHandle != NULL)
    delete pfHandle;
}

// Given a RID, return the record
void RM_FileHandle::getRec (const RID &rid, Record &rec) const{
  void invalid = isValid(); if(invalid) return invalid;
  if(!this->isValidRID(rid))
    return RM_BAD_RID;
  PageNum p;
  SlotNum s;
  rid.getPageNum(p);
  rid.getSlotNum(s);
  void rc = 0;
  PF_PageHandle ph;
  RM_PageHdr page_hdr(this->getNumSlots());
  if((rc = pfHandle->getThisPage(p, ph)) ||
     // Needs to be called everytime getThisPage is called.
     (rc = pfHandle->unpinPage(p)) ||
     (rc = this->getPageHeader(ph, page_hdr))
    )
    return rc;
  BitMap b(page_hdr.free_slot_map, this->getNumSlots());

  if(b.test(s)) // already free
    return RM_NORECATRID;

  char * p_data = NULL;
  if(void rc = this->getSlotPointer(ph, s, p_data))
    return rc;

  rec.set(p_data, hdr.ext_record_size, rid);
  return 0;
}


void RM_FileHandle::insertRec  (const char *p_data, RID &rid){
  void invalid = isValid(); if(invalid) return invalid;
  if(p_data == NULL)
    return RM_NULLRECORD;

  PF_PageHandle ph;
  RM_PageHdr page_hdr(this->getNumSlots());
  PageNum p;
  SlotNum s;
  void rc;
  char * pSlot;
  if((rc = this->getNextFreeSlot(ph, p, s)))
    return rc;
  if((rc = this->getPageHeader(ph, page_hdr)))
    return rc;
  BitMap b(page_hdr.free_slot_map, this->getNumSlots());
  // std::cerr << "RM_FileHandle::insertRec befor" << b << std::endl;
  // TODO GetSlotPtr is trashing the page_hdr
  if((rc = this->getSlotPointer(ph, s, pSlot)))
    return rc;
  rid = RID(p, s);
  memcpy(pSlot, p_data, this->fullRecordSize());
  
  b.reset(s); // slot s is no longer free
  page_hdr.num_of_free_slots--;
  if(page_hdr.num_of_free_slots == 0) {
    // remove from free list 
    hdr.first_free_page = page_hdr.next_free_page;
    page_hdr.next_free_page = RM_PAGE_FULLY_USED;
  }
  // std::cerr << "RM_FileHandle::insertRec num_of_free_slots in page " << page_hdr.num_of_free_slots << std::endl;
  b.toCharBuf(page_hdr.free_slot_map, b.numChars());
  rc = this->setPageHeader(ph, page_hdr);
  // std::cerr << "RM_FileHandle::insertRec after" << b << std::endl;
  return 0;
}

void RM_FileHandle::deleteRec  (const RID &rid){
  void invalid = isValid(); if(invalid) return invalid;
  if(!this->isValidRID(rid))
    return RM_BAD_RID;
  PageNum p;
  SlotNum s;
  rid.getPageNum(p);
  rid.getSlotNum(s);
  void rc = 0;
  PF_PageHandle ph;
  RM_PageHdr page_hdr(this->getNumSlots());
  if((rc = pfHandle->getThisPage(p, ph)) ||
     (rc = pfHandle->markDirty(p)) ||
     // Needs to be called everytime getThisPage is called.
     (rc = pfHandle->unpinPage(p)) ||
     (rc = this->getPageHeader(ph, page_hdr))
    )
    return rc;

  BitMap b(page_hdr.free_slot_map, this->getNumSlots());

  if(b.test(s)) // already free
    return RM_NORECATRID;

  // TODO considering zero-ing record - IOs though
  b.set(s); // s is now free
  if(page_hdr.num_of_free_slots == 0){
    // this page used to be full and used to not be on the free list
    // add it to the free list now.
    page_hdr.next_free_page = hdr.first_free_page;
    hdr.first_free_page = p;
  }
  page_hdr.num_of_free_slots++;
  // std::cerr << "RM_FileHandle::deleteRec num_of_free_slots in page "
  //           << page_hdr.num_of_free_slots
  //           << std::endl;
  b.toCharBuf(page_hdr.free_slot_map, b.numChars());
  rc = this->setPageHeader(ph, page_hdr);
  return rc;
}

// TODO write insert in terms of update
void RM_FileHandle::updateRec  (const Record &rec){
  void invalid = isValid(); if(invalid) return invalid;
  RID rid;
  rec.getRID(rid);
  PageNum p;
  SlotNum s;
  rid.getPageNum(p);
  rid.getSlotNum(s);

  if(!this->isValidRID(rid))
    return RM_BAD_RID;

  PF_PageHandle ph;
  char * pSlot;
  void rc;
  RM_PageHdr page_hdr(this->getNumSlots());
  if((rc = pfHandle->getThisPage(p, ph)) ||
     (rc = pfHandle->markDirty(p)) ||
     // Needs to be called everytime getThisPage is called.
     (rc = pfHandle->unpinPage(p)) ||
     (rc = this->getPageHeader(ph, page_hdr))
    )
    return rc;

  BitMap b(page_hdr.free_slot_map, this->getNumSlots());

  if(b.test(s)) // free - cannot update
    return RM_NORECATRID;

  char * p_data = NULL;
  rec.getData(p_data);

  if(void rc = this->getSlotPointer(ph, s, pSlot))
    return rc;
  memcpy(pSlot, p_data, this->fullRecordSize());
  return 0;
}



// Forces a page (along with any contents stored in this class)
// from the buffer pool to disk.  Default value forces all pages.
void RM_FileHandle::forcePages (PageNum page_num){
  void invalid = isValid(); if(invalid) return invalid;
  if(!this->isValidPageNum(page_num) && page_num != ALL_PAGES)
    return RM_BAD_RID;
  return pfHandle->forcePages(page_num);
}

// isValidPageNum
// Desc: Internal.  Return TRUE if page_num is a valid page number
//       in the file, FALSE otherwise
// In:   page_num - page number to test
// Ret:  TRUE or FALSE
bool RM_FileHandle::isValidPageNum(const PageNum page_num) const{
   return (bool_file_open &&
         page_num >= 0 &&
         page_num < hdr.num_of_pages);
}

bool RM_FileHandle::isValidRID(const RID rid) const{
  PageNum p;
  SlotNum s;
  rid.getPageNum(p);
  rid.getSlotNum(s);

  return isValidPageNum(p) &&
    s >= 0 &&
    s < this->getNumSlots();
}

void RM_FileHandle::isValid() const{
  if((pfHandle == NULL) || !bool_file_open)
    return RM_FNOTOPEN;
  if(getNumSlots() <= 0)
    return RM_RECSIZEMISMATCH;
  return 0;
}
