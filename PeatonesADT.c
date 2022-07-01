//
// Created by MateoPérezdeGracia on 6/29/2022.
//

#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DAY_MAX_LETTERS 10
#define MAX_DATE 20
#define MAX_NAME 25
typedef struct reading {

    //char * month ;    no se usan
    //int mDate;
    int year;
    char dateFormatted[MAX_DATE];  // el main tiene que pasar la feche formateada
    //char day[DAY_MAX_LETTERS];
    //int sensorId;
    int time;      //  [0,23]T
    int counts;
} TReading;

typedef struct sensor{
    int id;
    char * name;
    //char status;
    TReading * maxCount;
    long int sensorCounts;
} TSensor;

typedef struct day {
    int daylightCount;
    int nightCount;
}TDay;

typedef struct NodeYear{
    int year;
    long int yearCount;
    //struct NodeYear * first;
    struct NodeYear * next;
}TNodeYear;

typedef TNodeYear * TYearList;

typedef struct peatonesCDT {
    size_t sensorsSize;
    TSensor ** sensorsVec;  // vector dinámico de punteros TSensor
    TYearList first;  // list
    TYearList next;  // list
    TDay dayVec[CANT_DAYS];
} peatonesCDT;

//static void freeYearRec(TYearList first);

peatonesADT newPeatones(void){
    return calloc(1, sizeof(peatonesCDT));
}

void freeRecList(TYearList list){
    if(list==NULL) return;
    freeRecList(list->next);
    free(list);
}
void freeVec(TSensor ** sensors, size_t dim){
    for(int i=0; i<dim; i++){
        free(sensors[i]->name);
        free(sensors[i]->maxCount);
        free(sensors[i]);
    }
    free(sensors);
}

void freePeatones(peatonesADT pea){
    freeRecList(pea->first);
    freeVec(pea->sensorsVec, pea->sensorsSize);
    free(pea);
}
int putSensor(peatonesADT pea, int id, char * name){
    if(id > pea->sensorsSize){
        // si no tengo espacio en el vector, agrando sensorVec.
        pea->sensorsVec = realloc(pea->sensorsVec, id * sizeof(pea->sensorsVec[0]));
        for(int i=pea->sensorsSize; i < id; i++){
            pea->sensorsVec[i]= NULL;
        }
        pea->sensorsSize = id;
    }
    TSensor * sensor= malloc(sizeof(*sensor));
    sensor->id = id;
    sensor->sensorCounts=0;
    sensor->name = malloc(strlen(name) + 1); //hacer funcion copy auxiliar
    strcpy(sensor->name, name);
    sensor->maxCount = calloc(1, sizeof(*(sensor->maxCount)));
    pea->sensorsVec[id-1]=sensor;
    return 1;  // podría ser void esta funcion, xq
}
int sensorExists(peatonesADT pea, int id){
    return (pea->sensorsSize > id-1 && pea->sensorsVec[id-1]!=NULL);
}
char* getNameById(peatonesADT pea, int sensorID){
    if(  !sensorExists(pea, sensorID) ) return NULL;
    return pea->sensorsVec[sensorID-1]->name;
}





