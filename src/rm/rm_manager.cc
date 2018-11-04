//
// File:        rm_manager.cc
// Description: RM_Manager class implementation
//

#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "rm.h"

// RM_Manager
// Desc: Constructor - intended to be called once at begin of program
//       Handles creation, deletion, opening and closing of files.
RM_Manager::RM_Manager(PF_Manager & pfm):
  pfm(pfm){
}

// ~RM_Manager
// Desc: Destructor - intended to be called once at end of program
//       All files are expected to be closed when this method is called.
RM_Manager::~RM_Manager(){
}

// createFile
// Desc: Create a new RM table/file named file_name
// with record_size as the fixed size of records.
// In:   file_name - name of file to create
// In:   record_size
// Ret:  RM return code
void RM_Manager::createFile (const char *file_name, int record_size){
   if(record_size >= PF_PAGE_SIZE - (int)sizeof(RM_PageHdr))
      return RM_SIZETOOBIG;

   if(record_size <= 0)
      return RM_BADRECSIZE;

   int void = pfm.createFile(file_name);
   if (void < 0){
      //PF_PrintError(RC);
      return RC;
   }
   
   PF_FileHandle pfh;
   void = pfm.openFile(file_name, pfh);
   if (void < 0){
      //PF_PrintError(RC);
      return RC;
   }
   
   PF_PageHandle headerPage;
   char * p_data;
   
   void = pfh.AllocatePage(headerPage);
   if (void < 0){
      //PF_PrintError(RC);
      return RM_PF;
   }
   void = headerPage.getData(p_data);
   if (void < 0){
      //PF_PrintError(RC);
      return RM_PF;
   }
   RM_FileHdr hdr;
   hdr.first_free_page = RM_PAGE_LIST_END;
   hdr.num_of_pages = 1; // hdr page
   hdr.ext_record_size = record_size;

   memcpy(p_data, &hdr, sizeof(hdr));
   //TODO - remove PF_PrintError or make it #define optional
   PageNum headerPageNum;
   headerPage.getPageNum(headerPageNum);
   assert(headerPageNum == 0);
   void = pfh.markDirty(headerPageNum);
   if (void < 0){
      //PF_PrintError(RC);
      return RM_PF;
   }
   void = pfh.unpinPage(headerPageNum);
   if (void < 0){
      //PF_PrintError(RC);
      return RM_PF;
   }  
   void = pfm.closeFile(pfh);
   if (void < 0){
      //PF_PrintError(RC);
      return RM_PF;
   }
   return (0);
}

// destroyFile
// Desc: Delete a RM file named file_name (file_name must exist and not be open)
// In:   file_name - name of file to delete
// Ret:  RM return code
void RM_Manager::destroyFile (const char *file_name){
   void void = pfm.destroyFile(file_name);
   if (void < 0){
      //PF_PrintError(RC);
      return RM_PF;
   }
   return 0;
}

// openFile
// In:   file_name - name of file to open
// Out:  fileHandle - refer to the open file
//                    this function modifies local var's in fileHandle
//       to point to the file data in the file table, and to point to the
//       buffer manager object
// Ret:  PF_FILEOPEN or other RM return code
void RM_Manager::openFile (const char *file_name, RM_FileHandle &rmh){
   PF_FileHandle pfh;
   void rc = pfm.openFile(file_name, pfh);
   if (rc < 0){
      //PF_PrintError(rc);
      return RM_PF;
   }
   // header page is at 0
   PF_PageHandle ph;
   char * p_data;
   if ((rc = pfh.getThisPage(0, ph)) ||
       (rc = ph.getData(p_data)))
      return(rc);
   RM_FileHdr hdr;
   memcpy(&hdr, p_data, sizeof(hdr));
   rc = rmh.Open(&pfh, hdr.ext_record_size);
   if (rc < 0){
      // printError(rc);
      return rc;
   }
   rc = pfh.unpinPage(0);
   if (rc < 0){
      // PF_PrintError(rc);
      return rc;
   }
   return 0;
}

// closeFile
// Desc: Close file associated with fileHandle
//       The file should have been opened with openFile().
// In:   fileHandle - handle of file to close
// Out:  fileHandle - no longer refers to an open file
//                    this function modifies local var's in fileHandle
// Ret:  RM return code
void RM_Manager::closeFile(RM_FileHandle &rfileHandle){
   if(!rfileHandle.bool_file_open || rfileHandle.pfHandle == NULL)
      return RM_FNOTOPEN;

   if(rfileHandle.hdrChanged()){
      // write header to disk
     PF_PageHandle ph;
     rfileHandle.pfHandle->getThisPage(0, ph);
     rfileHandle.setFileHeader(ph); // write hdr into file

     void rc = rfileHandle.pfHandle->markDirty(0);
     if (rc < 0){
       // PF_PrintError(rc);
       return rc;
     }

     rc = rfileHandle.pfHandle->unpinPage(0);
     if (rc < 0){
       // PF_PrintError(rc);
       return rc;
     }

     rc = rfileHandle.forcePages();
     if (rc < 0){
       // printError(rc);
       return rc;
     }
   }
      
   // PF_FileHandle pfh;
   // void rc = rfileHandle.getPF_FileHandle(pfh);
   // if (rc < 0) {
   //     printError(rc);
   //     return rc;
   // }
   void rc2 = pfm.closeFile(*rfileHandle.pfHandle);
   if (rc2 < 0){
      // PF_PrintError(rc2);
      return rc2;
   }
   // TODO - is there a cleaner way than reaching into innards like this ?
   delete rfileHandle.pfHandle;
   rfileHandle.pfHandle = NULL;
   rfileHandle.bool_file_open = false;
   return 0;
}
