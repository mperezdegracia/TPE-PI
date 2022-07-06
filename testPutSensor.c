#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "errno.h"
 int main() {
    //TEST PUT SENSOR, SENSOR EXISTS, GETNAMEBY ID.
    errno=0;
    peatonesADT tad = newPeatones();
    putSensor(tad, 30, "LONDON");
    for(int id=1; id<20; id++){
        if(putSensor(tad, id, "LONDON")==ENOMEM) return ENOMEM;
        assert(sensorExists(tad, id)!=0);
        char * name = getNameById(tad, id );
        assert(strcmp(name,"LONDON")==0);
        printf("%d | name: %s \n", id, name);
    }

    freePeatones(tad);
    return 0;
}