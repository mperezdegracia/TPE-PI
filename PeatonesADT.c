//
// Created by MateoPérezdeGracia on 6/29/2022.
//

#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define BLOCK 10
#define DAY_MAX_LETTERS 10
#define MAX_DATE 20
#define MAX_NAME 25

typedef struct reading {

    //char * month ;    no se usan
    //int mDate;
    int year;
    int dateFormatted[DATE_FIELDS];  // el main tiene que pasar la feche formateada
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
    struct NodeYear * tail;
}TNodeYear;

typedef TNodeYear * TYearList;

typedef struct peatonesCDT {
    size_t sensorsSize;
    TSensor ** sensorsVec;  // vector dinámico de punteros TSensor
    TYearList first;  // list
    TYearList next;  // list
    TDay dayVec[CANT_DAYS];
} peatonesCDT;

peatonesADT newPeatones(void){
    return calloc(1, sizeof(peatonesCDT));
}//TESTED

static void freeRecList(TYearList list){
    if(list==NULL) return;
    freeRecList(list->tail);
    free(list);
}//TESTED

static void freeVec(TSensor ** sensors, size_t dim){
    for(int i=0; i<dim; i++){
        free(sensors[i]->name);
        free(sensors[i]->maxCount);
        free(sensors[i]);
    }
    free(sensors);
}//TESTED

void freePeatones(peatonesADT pea){
    freeRecList(pea->first);
    freeVec(pea->sensorsVec, pea->sensorsSize);
    free(pea);
}//TESTED

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
}//TESTED

int sensorExists(peatonesADT pea, int id){
    return (pea->sensorsSize > id-1 && pea->sensorsVec[id-1]!=NULL);
}//TESTED

char* getNameById(peatonesADT pea, int sensorID){
    if(  !sensorExists(pea, sensorID) ) return NULL;
    return pea->sensorsVec[sensorID-1]->name;
} //TESTED

long int getDailyCount(peatonesADT pea, char day, char option){
    //partiendo de que los parametros day y option son válidos se devuelve el campo que almacena los resultados totales de peatones por día y según el horario
    if (option){
        return pea->dayVec[(int)day].nightCount;
    }
    else
        return pea->dayVec[(int)day].daylightCount;
} //NEEDS addReading BEFORE TESTING

int * getSensorIDs(peatonesADT pea, int * dim){
    int i, j;
    int * sensorIDs = NULL;
    errno = 0; //seteo errno para luego verificar si hubo errores al reallocar memoria

    for ( i = 0, j = 0; i < pea->sensorsSize; i++){
        //reservo posiciones del vector de a bloques
        if (j % BLOCK == 0){
            sensorIDs = realloc(sensorIDs, (j+ BLOCK) * sizeof(int));
            if(sensorIDs == NULL || errno != 0 ){
                printf("Error: %s\n", strerror(errno));
                exit(1);
            }
        }
        //si hay un sensor en la posición que estoy revisando quiero quedarme con el id
        if (pea->sensorsVec[i] != NULL){
            sensorIDs[j++] = i+1;//se hace el fixeo basandose en que el valor de los ids está "corrido" un lugar respecto de los indices del vec (id=1 -> pos=0)
        }
    }
    *dim = j;
    return sensorIDs;
}//TESTED

long int getSensorCount(peatonesADT pea, int sensorID){
    return pea->sensorsVec[sensorID-1]->sensorCounts;
}//TESTED

int hasNextYear(peatonesADT pea){
    return pea->next != NULL;
} //NEEDS addReading BEFORE TESTING

static compareInt(int num1, int num2){
  return num1 - num2;
}

static int getYearRec(TYearList list, int year){
    if(list==NULL || compareInt(list->year, year) < 0) return -1;
    if(list->year == year){
        return year;
    }
    return getYearRec(list->tail, year);
} //NEEDS addReading BEFORE TESTING

long int getYearCount(peatonesADT pea, int year){
    return getYearRec(pea->first, year);
} //NEEDS addReading BEFORE TESTING

void toBeginYear(peatonesADT pea){
    pea->next = pea->first;
} //NEEDS addReading BEFORE TESTING (POSSIBLE: CHECK NULL STATEMENT)

void nextYear(peatonesADT pea, long int * yearCount, int * year){
    assert(hasNextYear(pea));
    *yearCount = pea->next->yearCount;
    *year = pea->next->year;
    pea->next =  pea->next->tail;
}

int getMaxReadingById(peatonesADT pea, int id, int * maxCount, char ** name, int date[DATE_FIELDS]){
    if(!sensorExists(pea, id) || pea->sensorsVec[id-1]->maxCount == NULL) return 0; // chequeo si existe el sensor o si no hay una medida máxima
    *maxCount = pea->sensorsVec[id-1]->maxCount->counts;
    *name = pea->sensorsVec[id-1]->name;
    for (int field = HOUR; field < DATE_FIELDS ; field++) {
        date[field] = pea->sensorsVec[id-1]->maxCount->dateFormatted[field];
    }
    return 1;
} //NEEDS addReading BEFORE TESTING





