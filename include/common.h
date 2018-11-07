/*
 * File: common.h
 * Description: basic setting about page file
 * Author: Liu Chaoyang
 * E-mail: chaoyanglius@gmail.com
 *
 */
#pragma once

// maximum number of pages in buffer
#define PAGE_BUFFER_SIZE 40
#define PAGE_DATA_SIZE 4092
#define PAGE_WHOLE_SIZE 4096

//               segment table
//          _______________________
//          ______File Header______
//          ______Data segment_____
//          ______index segment____
//          ______long segment_____
//          ____rollback segment___
//          ______temp segment_____
//
// micro for segment table
#define DATA_SEGMENT_SIZE 4096 * 80
#define INDEX_SEGMENT_SIZE 4096 * 80 
#define LONG_SEGMENT_SIZE 4096 * 8
#define ROLLBACK_SEGMENT_SIZE 4096 * 80
#define TEMP_SEGMENT_SIZE 4096 * 80 

#define DATA_SEGMENT_OFFSET FILE_HEADER_SIZE
#define INDEX_SEGMENT_OFFSET DATA_SEGMENT_OFFSET + DATA_SEGMENT_SIZE
#define LONG_SEGMENT_OFFSET INDEX_SEGMENT_OFFSET + INDEX_SEGMENT_SIZE
#define ROLLBACK_SEGMENT_OFFSET LONG_SEGMENT_OFFSET + LONG_SEGMENT_SIZE
#define TEMP_SEGMENT_OFFSET ROLLBACK_SEGMENT_OFFSET + ROLLBACK_SEGMENT_SIZE

#define FILE_HEADER_SIZE 4096 + DATA_SEGMENT_SIZE / 8 + INDEX_SEGMENT_SIZE / 8 + LONG_SEGMENT_SIZE / 8 + ROLLBACK_SEGMENT_SIZE / 8 + TEMP_SEGMENT_SIZE / 8;

typedef int PageID_t;
