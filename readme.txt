Hanqi Yang (Sheldon)
V00998854
CSC360 Assignment 3

==================
File 1: diskinfo.c
==================
Retrieve information about the entire file system through the superblock and fatblock.

How to run:
./diskinfo subdirs.img


==================
File 2: disklist.c
==================

Starting from the root block's starting block, sequentially retrieve the values of each 64-byte file information and organize them for printing.
If there is a need to enter a subdirectory, compare the input file name and directory name to determine how to continue retrieving each 64-byte file information from the subdirectory's starting block.

How to run:
./disklist subdirs.img
./disklist subdirs.img /subdir1

==================
File 3: diskget.c
==================
Building upon part 2, 
modify the code to locate the 64-byte information of a specific file instead of a directory. 
Then, using the starting block and number of blocks for this file, identify the actual starting position. 
Copy the file byte by byte to the local computer, and if 512 bytes have been copied, 
use the File Allocation Table (FAT) to find the position of the next block, 
repeating this process until the entire file is copied.

./diskget subdirs.img foo.txt foo.txt
./diskget subdirs.img /subdir1/subdir2/foo2.txt foo2.txt

==================
File 4: diskput.c
==================
Building upon part 3, copy a file into the file system by placing each byte into the user-specified path. 
At the same time, confirm the position of the next block based on the FAT table. 
If the user-provided path does not exist, locate empty blocks in the FAT table. 
If empty blocks are found, note the starting block. 
Once a sufficient number of blocks are located, create the directory, and then copy the file into it.

How to run:
./diskput subdirs.img foo4.txt /subdir1/foo4.txt