//
// File:        rm_rid.h
// Description: The Record Id interface
//

#ifndef RM_RID_H
#define RM_RID_H

#include "redbase.h"
#include "iostream"
#include <cassert>
#include <cstring>

using namespace std;

// PageNum: uniquely identifies a page in a file
typedef int PageNum;

// SlotNum: uniquely identifies a record in a page
typedef int SlotNum;

// RID: Record id interface
class RID {
public:
  static const PageNum NULL_PAGE = -1;
  static const SlotNum NULL_SLOT = -1;
  RID()
    :page(NULL_PAGE), slot(NULL_SLOT) {}     // Default constructor
  RID(PageNum page_num, SlotNum slotNum)
    :page(page_num), slot(slotNum) {}
  ~RID(){}                                        // Destructor
  
  void getPageNum(PageNum &page_num) const          // Return page number
  { page_num = page; return 0; }
  void getSlotNum(SlotNum &slotNum) const         // Return slot number
  { slotNum = slot; return 0; }

  PageNum Page() const          // Return page number
  { return page; }
  SlotNum Slot() const          // Return slot number
  { return slot; }
  
  bool operator==(const RID & rhs) const{
    PageNum p;SlotNum s;
    rhs.getPageNum(p);
    rhs.getSlotNum(s);
    return (p == page && s == slot);
  }

private:
  PageNum page;
  SlotNum slot;
};

ostream& operator <<(ostream & os, const RID& r);

#endif // RM_RID_H
