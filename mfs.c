/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:  Richard Aguilar
*         Melisa Sever
*         Ryan Scott
*         Jonathan Valadez
*
* Student IDs: 977075554
*              921662115
*              921814228
*              922274961
*
* GitHub Name: raguilar0917
* Group Name: The Beerman Fan Club
* Project: Basic File System
*
* File: mfs.c
*
* Description: Implementation of basic file system 
    commands from mfs.h. The driver uses these to 
    interact with the system
*
**************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "mfs.h"
#include "b_io.h"
#include "directory_entry.h"
#include "fsLow.h"
#include "free_space_helpers.h"
#include "constants.h"


/*
    Creates a directory with the name of the last toke in pathname
    Ignores the mode, but could be used to set permissions

    Returns 0 on success, -1 on failure
*/
int fs_mkdir(const char *pathname, mode_t mode){
    parseData* data = parsePath(pathname);
    int ret_value = 0;
    
    // If path is valid but last token DNE, create new dir with last token name  
    if(data->testDirectoryStatus == 0 && data->dirPointer != NULL){
        DE* parent = malloc(vcb->size_of_block * data->dirPointer->d_reclen);

        LBAread(parent, data->dirPointer->d_reclen, data->dirPointer->directoryStartLocation);


        new_dir_data* d = DirectoryInit(parent);

        // True if maxdir has been reached. Resize parent to accomodate new dir
        if(d->newDir != NULL){

            parent = malloc(vcb->size_of_block * d->newDir->size);

            LBAread(parent, d->newDir->size, d->newDir->location);
            free(d->newDir);

        }
        // Rename new file to given name
        strncpy(parent[d->index].name, data->nameOfLastToken, MAX_DE_NAME);
        
        free(d);
        d = NULL;

        // Save changes
        LBAwrite(parent, parent[0].size, parent[0].location);

        free(parent);
        parent = NULL;
    }
    else if(data->testDirectoryStatus == 1 || data->testDirectoryStatus == 2){
            printf("Error: File already exists\n");
            ret_value = -1;

        }
    else {
        printf("Error: Invalid path\n");
        ret_value = -1;
    }


    free(data->dirPointer);
    free(data);
    data = NULL;
    
    return(ret_value);
    
};

int fs_rmdir(const char *pathname){
    DE* dir = malloc(vcb->root_size*vcb->size_of_block);
    int is_success = 1;

    
    if(dir == NULL){
        printf("rmdir failed to allocate memory.\n");
        is_success = 0;
    }

    parseData* dir_info = parsePath(pathname);

    if(dir_info->testDirectoryStatus != 1){
        printf("Path was not a directory\n");
        is_success = 0;
    }


    if(is_success == 1){
        LBAread(dir, dir_info->dirPointer->d_reclen, dir_info->dirPointer->directoryStartLocation);
        int files = numberFilesInDir(dir);

        if(files == 2){
            
            // Move to parent 
            LBAread(dir, dir[1].size, dir[1].location);

            char* name = "\0";
            strncpy(dir[dir_info->directoryElement].name, name, MAX_DE_NAME);


            MarkBlocksFree(dir[dir_info->directoryElement].location, dir[dir_info->directoryElement].size);

            LBAwrite(dir, dir[0].size, dir[0].location);      
  
        }
        else{
            printf("Could not remove directory: Directory was not empty.\n");
            is_success = 0;
        }

    }


    free(dir_info->dirPointer);
    free(dir_info);
    free(dir);

    dir_info = NULL;
    dir = NULL;

    return is_success;
};

/*
Conditions:
    1.)A file has to exist.

Steps:
    1.)Find file that exists & want to delete
    2.)Mark the blocks that the file was using as free
    3.)Set file to null
    4.)Mark the directory entry as unused.
*/
int fs_delete(char* filename){      //removes a file

    parseData *deleteFileData = parsePath(filename);

    int filePosition = deleteFileData->directoryElement; //gives me position where file is at.

    int specificElement = deleteFileData->directoryElement;
    DE *tempCheck = malloc(vcb->size_of_block * deleteFileData->dirPointer->d_reclen);
    
    
    
       // deleteFileData->dirPointer->d_reclen //give me size of current directory file is in.

        //read parent directory.
        LBAread(tempCheck, deleteFileData->dirPointer->d_reclen,
         deleteFileData->dirPointer->directoryStartLocation);


        char* name2 = "\0";
        strncpy(tempCheck[filePosition].name, name2, MAX_DE_NAME);

        
        //Mark blocks that the file was using as free.
        MarkBlocksFree(filePosition,tempCheck[filePosition].size);

        //Set file to null


        LBAwrite(tempCheck, deleteFileData->dirPointer->d_reclen,
         deleteFileData->dirPointer->directoryStartLocation);

        //Mark directory entry as unused
        //deleteFileData->directoryElement = 0;


        free(deleteFileData->dirPointer);
        free(deleteFileData);
        free(tempCheck);
        return 0;
};	

// Directory iteration functions
fdDir * fs_opendir(const char *pathname){
    char* path = malloc(MAX_PATH_LENGTH);
    strncpy(path, pathname, MAX_PATH_LENGTH);

    //Parse data and get information
    if(fs_isDir(path) == 0){ printf("\nfs_opendir ERROR: Pathname is not a directory\n"); return NULL;}

   // printf("About to parse in open\n");
    parseData* parsed_data = parsePath(path);
   // printf("Finished parse in open\n");
    unsigned short d_reclen = parsed_data->dirPointer->d_reclen;
    unsigned short dirEntryPosition = parsed_data->dirPointer->dirEntryPosition;
    uint64_t directoryStartLocation = parsed_data->dirPointer->directoryStartLocation;

    DE* directoryPtr = malloc(d_reclen*vcb->size_of_block);
    LBAread(directoryPtr, d_reclen, directoryStartLocation);


    // Make a copy of fdDir in parsed_data
    fdDir* fdPtr = malloc(sizeof(fdDir));
    if(fdPtr == NULL){
        printf("Error: Failed to allocate memory\n");

        return NULL;
    }
    fdPtr->directoryStartLocation = directoryPtr->location;
    fdPtr->d_reclen = directoryPtr->size;
    fdPtr->dirEntryPosition = 0;
    fdPtr->ii = malloc(sizeof(struct fs_diriteminfo));
    if(fdPtr->ii == NULL){
        printf("Error: Failed to allocate memory\n");

        return NULL;
    }
    
    //free stuff
    free(parsed_data->dirPointer);
    free(parsed_data);
    free(directoryPtr);
    free(path);

    return fdPtr;
};

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
   // printf("In readdir\n");
    int foundItem = 0;

    
    DE* directory = malloc(dirp->d_reclen * vcb->size_of_block);
    LBAread(directory, dirp->d_reclen, dirp->directoryStartLocation);

    // Needs to be freed from previous call before updating with new info
    free(dirp->ii);
    dirp->ii = NULL;
    dirp->ii = malloc(sizeof(struct fs_diriteminfo));
    if(dirp->ii == NULL){
        printf("Error: Failed to allocate memory\n");

        return NULL;
    }
    for(int i = dirp->dirEntryPosition; i < ((directory->size*vcb->size_of_block)/(sizeof(DE))); i++){
        // Check all but empty dirs
        if(strcmp(directory[i].name, "\0") != 0){
            strncpy(dirp->ii->d_name, directory[i].name, MAX_DE_NAME);
            dirp->ii->fileType = directory[i].is_directory; //have something to tell you file type
            dirp->dirEntryPosition = i + 1;
            foundItem = 1;
            break;
        }
    }


    free(directory);
    
    if(foundItem == 0){
        return NULL;
    }

    return dirp->ii;
};

int fs_closedir(fdDir *dirp){
    free(dirp->ii);
    free(dirp);

    return 1;
};

// Misc directory functions

/*
    char* pathname is char buffer to copy to
    size_t size is size of buffer
*/
char * fs_getcwd(char *pathname, size_t size){
    strncpy(pathname, current_working_dir, size);

    return pathname;
};


/*
    Uses strtok to count num tokens in path
    Returns number of tokens
*/

int countPathTokens(char* pathname){
    int numTokens = 0;

    char* pathCopy = malloc(MAX_PATH_LENGTH);
    strncpy(pathCopy, pathname, MAX_PATH_LENGTH);


    char* token = strtok(pathCopy, "/");

    while(token != NULL){
        numTokens++;

        token = strtok(NULL, "/");
    }

    
    free(pathCopy);
    pathCopy = NULL;

    return numTokens;
}

/*
    Takes valid path and formats to be in line with what we expect
    Ex. Remove extra '/', resolve and remove '.' and '..' 

    Returns buffer containing formatted path
*/

char* formatPath(char *pathname){
    int numTokens = countPathTokens(pathname);

    /* 
        Array of char to track which tokens should be in final path
        1 for add token, 0 for ignore

        ith value set to 0 if '.' is found
        (i-1)th value set to 0 if ".." is found
            This can cascade with multiple ".." in a row. 
    */
    char* tokenTracker = malloc(sizeof(char*)* (numTokens));
    for(int i=0; i < numTokens; i++){
        tokenTracker[i] = 1;
    }

    // Current position in tokenTracker
    int tokenIter = 0;


    // Return path to be manipulated
        // In the end, will represent an absolute path
    char* newPath = malloc(MAX_PATH_LENGTH);
    if(newPath == NULL){
        printf("Error: Failed to allocate memory\n");

        return NULL;
    }
    newPath[0] = '/';
    newPath[1] = '\0';
    int pathOffset = 1; // Offset from start of newPath
        // Set to overwrite null char

    // Copy of path to gather tokens from
    char* pathCopy = malloc(MAX_PATH_LENGTH);
    strncpy(pathCopy, pathname, MAX_PATH_LENGTH);
    
    if(pathname[0] != '/'){
      //  printf("Formatting relative\n");
    }

    char* currentToken = strtok(pathCopy, "/");
    char* nextToken;


    /*

        /A/B/C/../C/../../B/E/F/../G
        Should be /A/B/E/G
        So final is 
        100000011001
        111111111111
        
        Token iter is 3
        111 See .., set 1100 (now have 1 previousDir, set 3 and 3-1 to 0)
        Token iter is 5
        11001 see .., set 110000 (now 1, set 5 and 5-1 to 0)
        Token iter is 6
        110000 see.., set (now 2, set 6 and 6-2 to 0)
    */

    /*
        Resolves "." and ".." by marking associated tokens 
        as unneeded, so we don't add them to the path later

    */
    while(currentToken != NULL){
        if(strcmp(currentToken, ".\0") == 0){
            tokenTracker[tokenIter] = 0;

        }
        else if(strcmp(currentToken, "..\0") == 0){

            // Set tokenIter and last 1 to 0
            tokenTracker[tokenIter] = 0;
            for(int i=tokenIter-1; i >= 0; i--){
                if(tokenTracker[i] == 1){
                    tokenTracker[i] = 0;
                    break;
                }
            }
        }

        tokenIter++;
        currentToken = strtok(NULL, "/");

    }


    // Reset strtok and iterator
    strncpy(pathCopy, pathname, MAX_PATH_LENGTH);
  //  printf("Pathcopy is %s right now\n", pathCopy);
    currentToken = strtok(pathCopy, "/");
    tokenIter = 0;
    /*
        Adds ith token to path if ith entry in tokenTracker == 1
    */
    while(currentToken != NULL){
        if(tokenTracker[tokenIter] == 1){
  
            strncpy(newPath+pathOffset, currentToken, MAX_PATH_LENGTH-pathOffset);
            // Note that token contains null char, which we copy, yet path offset increment 
                // does not include it. We'll overwrite that null char on each copy
            
            
            pathOffset+=strlen(currentToken);
            newPath[pathOffset] = '/';
            newPath[pathOffset+1] = '\0';
            pathOffset++;

        }
        

        currentToken = strtok(NULL, "/");
        tokenIter++;

    }

    free(tokenTracker);
    free(pathCopy);
    tokenTracker = NULL;
    pathCopy = NULL;
   
    return newPath;
}


/*
    Uses given path name to adjust current_working_directory

    Returns 0 on success, else -1

*/
int fs_setcwd(char *pathname){       //linux chdir
    parseData* data = parsePath(pathname);
    int ret_val = 0;
    int is_relative = 0;

    
    if(data->testDirectoryStatus == 2){
        printf("Error: Not a directory\n");
        ret_val = -1;
    }
    else if(data->testDirectoryStatus == 0){
        printf("Error: Invalid path\n");
        ret_val = -1;
    }
    else{
        
        // Holds full new cwd
        char* newPath;

        // Handle relative or absolute
        if(pathname[0] != '/'){

            // Get CWD
            char* cwd = malloc(MAX_PATH_LENGTH);
            if(cwd == NULL){
                printf("Error: Failed to allocate memory\n");

                return -1;
            }
            fs_getcwd(cwd, MAX_PATH_LENGTH);


            // Add cwd to path, then tack on the relative path after it
                // Redundancy for clarity. Imagine starting at 0 and rebuilding
            strncpy(current_working_dir, cwd, MAX_PATH_LENGTH);
            strncpy(current_working_dir+strlen(cwd), pathname, MAX_PATH_LENGTH-strlen(cwd));

            // Format the new full path to remove unnecessary tokens and chars
            newPath = formatPath(current_working_dir);

            // Overwrite with tweaked path
            strncpy(current_working_dir, newPath, MAX_PATH_LENGTH);
            
            free(cwd);
            cwd = NULL;
        }
        else{

            // Format the given path, then save
            newPath = formatPath(pathname);
            strncpy(current_working_dir, newPath, MAX_PATH_LENGTH);
        }


        
        free(newPath);
        newPath = NULL;
        

    }


    free(data->dirPointer);
    free(data);
    data = NULL;
    return ret_val;
}; 
 
int fs_isFile(char * filename){     //return 1 if file, 0 otherwise
    int isFileValue = 0;
    parseData *isFileData = parsePath(filename);
    if(isFileData == NULL){
        printf("Error: Failed to allocate memory\n");

        return 0;
    }

    if(isFileData->testDirectoryStatus == 2){
        isFileValue = 1;    //This is a file
        
    }
    else{
        isFileValue = 0;    //Not a file
        
    }
    
    free(isFileData->dirPointer);
    free(isFileData);   //free in relation to fact that parsePath malloc'd

    return isFileValue;
}
	
int fs_isDir(char * pathname){      //return 1 if directory, 0 otherwise
    int isDirRet = 0;
    parseData *isDirData = parsePath(pathname);
    if(isDirData == NULL){
        printf("Error: Failed to allocate memory\n");

        return 0;
    }

    if(isDirData->testDirectoryStatus == 1){
      //  printf("This is a directory\n");
        isDirRet = 1; //Is Directory
        
    }
    else{
        //printf("This is not a directory\n");
        isDirRet = 0;  //Is not a Directory
        
    }

    free(isDirData->dirPointer);
    free(isDirData);   //free in relation to fact that parsePath malloc'd

    return isDirRet;
}
	




int fs_stat(const char* pathname, struct fs_stat* buf){
    char* path = malloc(MAX_PATH_LENGTH);
    strncpy(path, pathname, MAX_PATH_LENGTH);

    parseData* parsed_data = parsePath(path);

    DE* directory = malloc(parsed_data->dirPointer->d_reclen * vcb->size_of_block);
    LBAread(directory, parsed_data->dirPointer->d_reclen, parsed_data->dirPointer->directoryStartLocation);

    
    buf->st_blksize = vcb->size_of_block;

    // If path is a file, dirPointer is the parent, so access data by index
    if(parsed_data->testDirectoryStatus == 2){
        buf->st_blocks = directory[parsed_data->directoryElement].size;
        buf->st_size = directory[parsed_data->directoryElement].size_bytes;
        buf->st_createtime = directory[parsed_data->directoryElement].creation_date;
        buf->st_modtime = directory[parsed_data->directoryElement].last_modified;
    }
    else{
        buf->st_blocks = directory->size;
        buf->st_size = directory->size_bytes;
        buf->st_createtime = directory->creation_date;
        buf->st_modtime = directory->last_modified;
    }


    
    free(directory);
    free(parsed_data->dirPointer);
    free(parsed_data);
    free(path);


    return 1;
}




/*
abosolute -> want starting directory as our root.
want to read it or store root in global memory.

relative -> get current working directory.

*/
struct parseData *parsePath(const char *pathname){
    // Allocate buffer at root's size
        // This size may not match other directories. Will resize if needed
    DE* dirBuffer = malloc(vcb->root_size * vcb->size_of_block);
    char delim[] = "/";
    parseData* data = malloc(sizeof(parseData));
    int isEmptyPath = 0;

    // Init data elements
    data->directoryElement = -1;
    data->dirPointer = NULL;
    data->testDirectoryStatus = 0;
    


    if(pathname[0] == '\0'){
        isEmptyPath = 1;
    }

    char* pathBuffer = malloc(MAX_PATH_LENGTH);
    if(pathBuffer == NULL){
        printf("Error: Failed to allocate memory\n");

        return NULL;
    }
    
    //here for identification of absolute or relative
    // Starting at root, absolute path
    if(pathname[0] == '/'){
        LBAread(dirBuffer, vcb->root_size, vcb->root_starting_index);

        
    }
    // Relative path
    else{
        // Use CWD to construct the absolute path from the relative path
        parseData* relativeData = parsePath(fs_getcwd(pathBuffer, MAX_PATH_LENGTH));
        
        LBAread(dirBuffer, relativeData->dirPointer->d_reclen, relativeData->dirPointer->directoryStartLocation);

        
        free(relativeData->dirPointer);
        free(relativeData);
        relativeData = NULL;
    }

    /*
    after our initial / root recognition.
    copy the rest of our pathname array "string"
    to our buffer.
    */

    strncpy(pathBuffer, pathname, MAX_PATH_LENGTH);
    /*
    this would get the first root directory name after its /

    Example:
    A/B/C/D
    this current element would return A, once that strtok recongizes tha delim "/"
    */
    char* current_element = strtok(pathBuffer, delim);
   // printf("Current token is %s\n", current_element);
    
    /*
    this gets the rest of the characters in the given string
    referencing the prior string.

    Example:
    our prior string was "A", so it will take that reference and cotinue
    the next string which would be B after that delim is found.
    */
    char* next_element = strtok(NULL, delim);

    
    int indexOfSearch = -1;
    
    // Check first element in path
    if(current_element != NULL){
        strncpy(data->nameOfLastToken, current_element, MAX_DE_NAME);

        indexOfSearch = findFileInDirectory(dirBuffer, current_element);

        if(indexOfSearch != -1){
            // If it's a directory, we want to read into our dirBuffer
            if(dirBuffer[indexOfSearch].is_directory == 1){
                LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);

                // If this is the last element of the path, we ended in a directory
                if(next_element == NULL){
                    data->testDirectoryStatus = 1;
                    data->directoryElement = indexOfSearch;

                }
            }
            else{
                // If it wasn't a directory and it's the last elem, then must be a file
                if(next_element == NULL){

                    data->testDirectoryStatus = 2;
                    data->directoryElement = indexOfSearch;
                }
            }
        }
        else{
            // If we aren't at the last element when we find something that doesn't exist, 
                // path is invalid
            if(next_element == NULL){
   //             printf("Last element does not exist. If you're mkdir, this is good\n");
            }
            else{
                // Prevents us from giving dir info if path invalid
                free(dirBuffer);
                dirBuffer = NULL; 
            }
        }
    }
    // If first elem. is NULL and path isn't empty, we have path "/"
    else if(isEmptyPath == 0){ 
        LBAread(dirBuffer, vcb->root_size, vcb->root_starting_index);
        data->testDirectoryStatus = 1;

    }

    
    // Check the rest of the elements in the path, if they exist
        // Mostly duplicated from above, except while condition
    while(next_element != NULL && dirBuffer != NULL){
        current_element = next_element;
        strncpy(data->nameOfLastToken, current_element, 256);


        // Find index of this file
        indexOfSearch = findFileInDirectory(dirBuffer, current_element);

        next_element = strtok(NULL,delim);

        // If file exists
        if(indexOfSearch != -1){
            // If it's a directory, we want to read into our dirBuffer
            if(dirBuffer[indexOfSearch].is_directory == 1){
                LBAread(dirBuffer, dirBuffer[indexOfSearch].size, dirBuffer[indexOfSearch].location);

                // If this is the last element of the path, we ended in a directory
                if(next_element == NULL){
 //                   printf("Last element is a directory\n");
                    data->testDirectoryStatus = 1;
                    data->directoryElement = indexOfSearch;

                }
            }
            else{
                // If it wasn't a directory and it's the last elem, then must be a file
                if(next_element == NULL){
  //                  printf("Last element is a file\n");

                    data->testDirectoryStatus = 2;
                    data->directoryElement = indexOfSearch;
                }
            }
        }
        else{
            // If we aren't at the last element when we find something that doesn't exist, 
                // path is invalid
            if(next_element == NULL){
 //               printf("Last element does not exist. If you're mkdir, this is good\n");

            }
            else{
                // Prevents us from giving dir info if path invalid
                free(dirBuffer);
                dirBuffer = NULL; 
            }
            break;
        }
        
    }
   

    data->directoryElement = indexOfSearch;

    if(dirBuffer != NULL){
        fdDir* fd = malloc(sizeof(fdDir));
        fd->directoryStartLocation = dirBuffer->location;
        fd->d_reclen = dirBuffer->size;
        data->dirPointer = fd;

      
    }
    else{
        data->dirPointer = NULL;

    }


    free(pathBuffer);
    free(dirBuffer);


    return data;
}

/*
    Move given file or directory to destination directory

    
    Return 1 on success, else -1
*/

int fs_move(char* src, char* dest){

    parseData* source = parsePath(src);

    if(source->testDirectoryStatus == 0){
        printf("Error: Source to move does not exist.\n");
        free(source->dirPointer);
        free(source);

        return -1;
    }

    parseData* destination = parsePath(dest);

    if(destination->testDirectoryStatus != 1){
        printf("Error: Destination is invalid.\n");

        free(source->dirPointer);
        free(source);
        free(destination->dirPointer);
        free(destination);

        return -1;
    }

    

    DE* file; 
    DE* fileParent;
    
    // If path was file, then dirPointer points to the parent already
        // Use the parent to get the child file
    // If path was dir, then use child to get parent
    if(source->testDirectoryStatus == 2){
        fileParent = malloc(source->dirPointer->d_reclen * vcb->size_of_block);
        LBAread(fileParent, source->dirPointer->d_reclen, source->dirPointer->directoryStartLocation);
        file = malloc(fileParent[source->directoryElement].size * vcb->size_of_block);
        if(file == NULL){
            return -1;
        }
        LBAread(file, fileParent[source->directoryElement].size, fileParent[source->directoryElement].location);
    }
    
    else if(source->testDirectoryStatus == 1){
        file = malloc(source->dirPointer->d_reclen * vcb->size_of_block);
        LBAread(file, source->dirPointer->d_reclen, source->dirPointer->directoryStartLocation);
        fileParent = malloc(file[1].size * vcb->size_of_block);
        LBAread(fileParent, file[1].size, file[1].location);



    }
    DE* dir = malloc(destination->dirPointer->d_reclen * vcb->size_of_block);

    LBAread(dir, destination->dirPointer->d_reclen, destination->dirPointer->directoryStartLocation);

    
    int res = appendDEtoDir(fileParent, source->directoryElement, dir, file);

    if(res != 2){
        free(dir);

        // May need to resize fileParent here
    }


    strncpy(fileParent[source->directoryElement].name, "\0", MAX_DE_NAME);
    LBAwrite(fileParent, fileParent->size, fileParent->location);



    free(file);
    free(fileParent);
    free(source->dirPointer);
    free(source);
    free(destination->dirPointer);
    free(destination);
    return 0;
}

/*

    Returns 1 on success, 2 if target dest resized
    - If target dest resized, the dir pointer changed location,
        so the caller no longer knows where dir is to free.
        appendDEtoDir will free in this case
*/

int appendDEtoDir(DE* fileParent, int index, DE* dir, DE* file){
    int dest = findEmptyEntry(dir);
    int ret = 1;

    // DE was full, resize
    if(dest == -1){

        dir = resize(dir);

        ret = 2;

        dest = findEmptyEntry(dir);
    }


    dir[dest].location = fileParent[index].location;
    strncpy(dir[dest].name, fileParent[index].name, MAX_DE_NAME);
    dir[dest].creation_date = fileParent[index].creation_date;
    dir[dest].last_modified = fileParent[index].last_modified;
    dir[dest].size = fileParent[index].size;
    dir[dest].size_bytes = fileParent[index].size_bytes;
    dir[dest].is_directory = fileParent[index].is_directory;

    // Update file with new parent info
    file[1].location = dir->location;
    file[1].size = dir->size;
    file[1].size_bytes = dir->size_bytes;




    LBAwrite(dir, dir->size, dir->location);
    LBAwrite(file, file->size, file->location);


    if(ret == 2){
        free(dir);
    }
    return ret;
}

void initCWD(){
    current_working_dir = malloc(MAX_PATH_LENGTH);
    strncpy(current_working_dir, "/\0", 2);
}