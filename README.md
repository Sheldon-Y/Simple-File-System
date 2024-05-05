### Simple File System (C in VisualStudioCode)

> Developed a simple file system for .img files that supports creating, reading, writing, and deleting files and directories, including mounting and unmounting of the file system.

1. **File System Structure Design**:
   - **Functionality**: Design a file system structure for .img disk image files, including metadata, file allocation table, directory structure, and data blocks.
   - **Implementation Method**: Define the basic structure of the file system, consisting of superblocks (storing overall information about the file system), index nodes (inodes, storing file attributes and data block pointers), and data blocks (storing actual file data). Utilize bitmaps to manage the allocation of free data blocks and inodes.
2. **File Creation, Reading, Writing, and Deletion**:
   - **Functionality**: Implement basic file operations allowing users to create, read, write, and delete files within the file system.
   - **Implementation Method**: Manage file operations through inodes and data blocks. Allocate a free inode and data blocks when creating a file, perform read and write operations by locating the data block addresses in the inode, and release the inode and all associated data blocks when deleting a file.
3. **Directory Management**:
   - **Functionality**: Implement the creation, reading, and deletion of directories, enabling users to organize files within the file system.
   - **Implementation Method**: Treat directories as special types of files that store index information of files and subdirectories. When creating a new directory, allocate an inode and initialize it as a directory type, updating directory contents when files are added or removed.
4. **File System Mounting and Unmounting**:
   - **Functionality**: Support mounting the file system onto an operating system so that it can be recognized and utilized, as well as unmounting it.
   - **Implementation Method**: During mounting, read the superblock from the .img file and initialize the file system structure in memory. Ensure all data is written back to the disk and the superblock is updated upon unmounting.
5. **Error Handling and Data Integrity**:
   - **Functionality**: Handle potential file system errors, such as insufficient disk space or non-existent files, and ensure data integrity.
   - **Implementation Method**: Implement error detection and handling mechanisms, providing error returns for file operations. Use logging or other synchronization mechanisms to ensure data consistency and integrity in case of anomalies.
