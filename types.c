#include "./types.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


/*
   0   Item is a file
   1   Item is a directory
   2   Item is a CSO phone-book server
   3   Error
   4   Item is a BinHexed Macintosh file.
   5   Item is DOS binary archive of some sort.
       Client must read until the TCP connection closes.  Beware.
   6   Item is a UNIX uuencoded file.
   7   Item is an Index-Search server.
   8   Item points to a text-based telnet session.
   9   Item is a binary file!
*/



struct type types[] = {
        ".pdf", '9',
        ".jpg", 'I',
        ".GIF", 'G',
        ".c",   '0',
        ".txt", '0',
        ".h",   '0',
        ".png", 'I',
        ".tar", '9',
        ".bin" , '9',
        /* ADD MORE HERE */
};



char 
ret_type(char *buffer) {
        
        if (is_dir(buffer))
                return '1';


        char *ptr;
        ptr = strrchr(buffer, '.');

        if (ptr == NULL) {
                return DEFAULT_TYPE;
        } 

        for (int i = 0; i < (sizeof(types) / sizeof(types[0])); i++) {
                if (strcmp(ptr, types[i].extension) == 0)
                        return types[i].code;
        }
        return DEFAULT_TYPE;
}

int 
is_dir(char *buffer) {
        struct stat sb;
        if (stat(buffer, &sb) == -1) {
                return -1;
        }
        if(S_ISDIR(sb.st_mode)) {
                return 1; 
        } else {
                return 0;
        }
}
