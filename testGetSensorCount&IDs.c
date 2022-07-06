//
// Created by UrielArias on 1/7/2022.
//

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "PeatonesADT.h"
#include <stdlib.h>

int main(void){
    peatonesADT tad = newPeatones();
    for(int id=1; id<20; id++) {
        putSensor(tad, id, "Street");
        assert(getSensorCount(tad,id) == 0);
    }
    int size = getCantSensores(tad);
    int sensors[size];
    getSensorIDs(tad, sensors);
    assert(size == 19);
    for(int i=0; i<size; i++) {
        printf("sensor ID: %d\n", sensors[i]);
        assert(sensors[i] == i+1);
    }

    freePeatones(tad);
    printf("OK");
    return 0;
}
