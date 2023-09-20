#ifndef __TYPE_S_H
#define __TYPE_S_H

struct type {
        char *extension;
        char code;
};

extern struct type types[];
char ret_type(char *buffer);
int is_dir(char *buffer);


#define DEFAULT_TYPE '0'        /*binary */

#endif
