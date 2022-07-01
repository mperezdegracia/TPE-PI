#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
int main() {
    //TEST PUT SENSOR, SENSOR EXISTS, GETNAMEBY ID.
    peatonesADT tad = newPeatones();
    for(int id=1; id<20; id++){
        assert(putSensor(tad, id, "Casa")==1);
        assert(sensorExists(tad, id)!=0);
        char * name = getNameById(tad, id );
        assert(strcmp(name,"Casa")==0);
        printf("%d | name: %s \n", id, name);
    }

    freePeatones(tad);
    return 0;
}
