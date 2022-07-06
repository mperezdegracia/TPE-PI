//
// Created by MateoPérezdeGracia on 7/5/2022.
//

#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
char * create(){
    char * new = malloc(0xFFFFFFFF);
    return new;

}
int main(){
    errno = 0;

    char * test = create();
    if(errno == ENOMEM) printf("error de memoria");
    free(test);
    return 0;
}
