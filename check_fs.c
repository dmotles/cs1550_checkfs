/**
 * check_fs.c
 * A quick little program that scans your disk and prints out what's in each block.
 * Will skip empty blocks.
 *
 * Author: Dan Motles
 * License: Do as you please.
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

//size of a disk block
#define BLOCK_SIZE 512

//we'll use 8.3 filenames
#define MAX_FILENAME 8
#define MAX_EXTENSION 3
#define MAX_FILESTR "9" //MATCH the number with MAX_FILENAME (damn stupid preprocessor) (add 1 though)
#define MAX_EXTSTR "4" //Same, match with number above to make the preprocessor happy (add 1 though)

#define SSCAN_FMT "/%" MAX_FILESTR "[^/]/%" MAX_FILESTR "[^.].%" MAX_EXTSTR "s"

// Files
#define DIR_FILE ".directories"
#define BLOCK_FILE ".disk"

//How many files can there be in one directory?
#define MAX_FILES_IN_DIR (BLOCK_SIZE - (MAX_FILENAME + 1) - sizeof(int)) / \
  ((MAX_FILENAME + 1) + (MAX_EXTENSION + 1) + sizeof(size_t) + sizeof(long))

//How much data can one block hold?
#define MAX_DATA_IN_BLOCK (BLOCK_SIZE - sizeof(size_t) - sizeof(long))

// misc
#define NO_EXISTING_FD -1
#define OPERATION_FAILED -1L
#define DISK_FULL -2L

/******************************************************************************
 * DIRECTORY ENTRY STRUCT
 *****************************************************************************/
struct cs1550_directory_entry
{
  char dname[MAX_FILENAME + 1]; //the directory name (plus space for a nul)
  int nFiles;     //How many files are in this directory.
          //Needs to be less than MAX_FILES_IN_DIR

  struct cs1550_file_directory
  {
    char fname[MAX_FILENAME + 1]; //filename (plus space for nul)
    char fext[MAX_EXTENSION + 1]; //extension (plus space for nul)
    size_t fsize;     //file size
    long nStartBlock;   //where the first block is on disk
  } files[MAX_FILES_IN_DIR];    //There is an array of these
};

typedef struct cs1550_directory_entry cs1550_directory_entry;

/******************************************************************************
 * BLOCK ENTRY STRUCT
 *****************************************************************************/
struct cs1550_disk_block
{
  //Two choices for interpreting size:
  //  1) how many bytes are being used in this block
  //  2) how many bytes are left in the file
  //Either way, size tells us if we need to chase the pointer to the next
  //disk block. Use it however you want.
  size_t size;

  //The next disk block, if needed. This is the next pointer in the linked
  //allocation list
  long nNextBlock;

  //And all the rest of the space in the block can be used for actual data
  //storage.
  char data[MAX_DATA_IN_BLOCK];
};
typedef struct cs1550_disk_block cs1550_disk_block;


int main() {
	int i, blockid, not_empty;
	cs1550_disk_block tmp;
	char* cptr;
	int fd = open( BLOCK_FILE, O_RDONLY );
	blockid = 0;
	while( read( fd, &tmp, sizeof( cs1550_disk_block ) ) ){
		not_empty = 0;
		cptr = (char*)&tmp;
		for( i = 0; i < sizeof( cs1550_disk_block ); i++ ) {
			if( *cptr ) {
				not_empty = 1;
				break;
			}
			cptr++;
		}
		
		if ( not_empty ) {
			printf( "=============== BLOCK ID %d HAS DATA ==============\n", blockid );
			printf( "Block address: %d\n", lseek( fd, 0, SEEK_CUR ) - 512 );
			printf( "Next block: %d\n", tmp.nNextBlock );
			printf( "Size: %d\n", tmp.size );
			printf( "Data:\n" );
			cptr = tmp.data;
			for( i = 0; i < tmp.size; i++ ) {
				putchar( *cptr );
				cptr++;
			}
			printf("\n");
		}

		blockid++;
			
	}

	close( fd );
}
