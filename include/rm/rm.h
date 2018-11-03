//
// File:        rm.h
// Description: Record Manager component interface
//

#ifndef RM_H
#define RM_H

#include "redbase.h"
#include "rm_error.h"
#include "rm_rid.h"
#include "pf.h"
#include "predicate.h"

// RM_FileHdr: Header structure for files
struct RM_FileHdr {
  int first_free_page;    // first free page in the linked list
  int num_of_pages;       // number of pages in the file
  int ext_record_size;    // record size as seen by users
};

class BitMap {
public:
  BitMap(int num_of_bits);
  BitMap(char * buf, int num_of_bits); //deserialize from buf
  ~BitMap();

  void set(unsigned int bit_number);
  void set(); // set all bits to 1
  void reset(unsigned int bit_number);
  void reset(); // set all bits to 0
  bool test(unsigned int bit_number) const;

  int numChars() const; // return size of char buffer to hold BitMap
  int toCharBuf(char *, int len) const; //serialize content to char buffer
  int getSize() const { return size; } 
private:
  unsigned int size;
  char * buffer;
};

ostream& operator <<(ostream & os, const BitMap& b);

// RM_PageHdr: Header structure for pages
#define RM_PAGE_LIST_END -1         // end of list of free pages
#define RM_PAGE_FULLY_USED -2       // page is fully used with no free slots
// not a member of the free list
struct RM_PageHdr {
  int next_free_page;    // next_free_page can be any of these values:
                         //  - the number of the next free page
                         //  - RM_PAGE_LIST_END if this is last free page
                         //  - RM_PAGE_FULLY_USED if the page is not free
  char * free_slot_map;  // A BitMap that tracks the free slots within the page
  int num_of_slots;
  int num_of_free_slots;

RM_PageHdr(int num_of_slots) : num_of_slots(num_of_slots), num_of_free_slots(num_of_slots)
    { free_slot_map = new char[this->mapSize()];}

  ~RM_PageHdr()
    { delete [] free_slot_map; }

  int size() const 
    { return sizeof(next_free_page) + sizeof(num_of_slots) + sizeof(num_of_free_slots)
        + BitMap(num_of_slots).numChars()*sizeof(char); }
  int mapSize() const
    { return this->size() - sizeof(next_free_page)
        - sizeof(num_of_slots) - sizeof(num_of_free_slots);}
  int toBuf(char *& buf) const
    { 
      memcpy(buf, &next_free_page, sizeof(next_free_page));
      memcpy(buf + sizeof(next_free_page), &num_of_slots, sizeof(num_of_slots));
      memcpy(buf + sizeof(next_free_page) + sizeof(num_of_slots),
             &num_of_free_slots, sizeof(num_of_free_slots));
      memcpy(buf + sizeof(next_free_page) + sizeof(num_of_slots) + sizeof(num_of_free_slots),
             free_slot_map, this->mapSize()*sizeof(char));
      return 0;
    }
  int fromBuf(const char * buf)
    {
      memcpy(&next_free_page, buf, sizeof(next_free_page));
      memcpy(&num_of_slots, buf + sizeof(next_free_page), sizeof(num_of_slots));
      memcpy(&num_of_free_slots, buf + sizeof(next_free_page) + sizeof(num_of_slots),
             sizeof(num_of_free_slots));
      memcpy(free_slot_map,
             buf + sizeof(next_free_page) + sizeof(num_of_slots) + sizeof(num_of_free_slots),
             this->mapSize()*sizeof(char));
      return 0;
    }
};

// Record: RM Record interface
class Record {
  friend class RecordTest;
public:
  Record ();
  ~Record();

  // Return the data corresponding to the record.  Sets *p_data to the record contents.
  void getData(char *&p_data) const;

  // Sets data in the record for a fixed recordsize of size.
  // Real object is only available and usable at this point not after construction
  void set(char *p_data, int size, RID id);

  // Return the RID associated with the record
  void getRID (RID &rid) const;
private:
  int record_size;
  char * data;
  RID rid;
};

// RM_FileHandle: RM File interface
class RM_FileHandle {
  friend class RM_FileHandleTest;
  friend class FileScan;
  friend class RM_Manager;
public:
  RM_FileHandle ();
  void Open(PF_FileHandle*, int record_size);
  void setHdr(RM_FileHdr h) { hdr = h; return 0;}
  ~RM_FileHandle();

  // Given a RID, return the record
  void getRec(const RID &rid, Record &rec) const;
  void insertRec(const char *p_data, RID &rid);     // Insert a new record
  void deleteRec(const RID &rid);                   // Delete a record
  void updateRec(const Record &rec);                // Update a record

  // Forces a page (along with any contents stored in this class)
  // from the buffer pool to disk.  Default value forces all pages.
  void forcePages(PageNum page_num = ALL_PAGES);

  void getPF_FileHandle(PF_FileHandle &) const;
  bool hdrChanged() const { return bool_hdr_changed; }
  int fullRecordSize() const { return hdr.ext_record_size; }
  int getNumPages() const { return hdr.num_of_pages; }
  int getNumSlots() const;

  void isValid() const;
private:
  bool isValidPageNum (const PageNum page_num) const;
  bool isValidRID(const RID rid) const;

  // Return next free page or allocate one as needed.
  void getNextFreePage(PageNum& page_num);
  void getNextFreeSlot(PF_PageHandle& ph, PageNum& page_num, SlotNum&);
  void getPageHeader(PF_PageHandle ph, RM_PageHdr & page_hdr) const;
  void setPageHeader(PF_PageHandle ph, const RM_PageHdr& page_hdr);
  void getSlotPointer(PF_PageHandle ph, SlotNum s, char *& p_data) const;

  // write hdr member using a newly open file's header page
  void getFileHeader(PF_PageHandle ph);
  // persist header into the first page of a file for later
  void setFileHeader(PF_PageHandle ph) const;

  PF_FileHandle *pfHandle;                   // pointer to opened PF_FileHandle
  RM_FileHdr hdr;                            // file header
  bool bool_file_open;                       // file open flag
  bool bool_hdr_changed;                     // dirty flag for file hdr
};

// FileScan: condition-based scan of records in the file
class FileScan {
 public:
  FileScan();
  ~FileScan();

  void openScan  (const RM_FileHandle &fileHandle,
                AttrType   attr_type,
                int        attr_length,
                int        attr_offset,
                CompOp     comp_op,
                void       *value,
                ClientHint pin_hint = NO_HINT); // Initialize a file scan
  void getNextRec(Record &rec);               // Get next matching record
  void closeScan ();                             // Close the scan
  bool isOpen() const { return (bool_open && prmh != NULL && pred != NULL); }
  void resetState() { current = RID(1, -1); }
  void gotoPage(PageNum p);
  int getNumSlotsPerPage() const { return prmh->getNumSlots(); }
 private:
  Predicate * pred;
  RM_FileHandle * prmh;
  RID current;
  bool bool_open;
};

// RM_Manager: provides RM file management
class RM_Manager {
public:
  RM_Manager    (PF_Manager &pfm);
  ~RM_Manager   ();

  void createFile (const char *file_name, int record_size);
  void destroyFile(const char *file_name);
  void openFile   (const char *file_name, RM_FileHandle &fileHandle);

  void closeFile  (RM_FileHandle &fileHandle);

private:
  PF_Manager&   pfm; // A reference to the external PF_Manager
};

#endif // RM_H
