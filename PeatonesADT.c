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

typedef struct reading {
    //char * month ;    no se usan
    //int mDate;
    //int year;     ya esta en dateFormatted
    int dateFormatted[DATE_FIELDS];  // vector de dim 4 con el dia, mes, anio y hora
    //char day[DAY_MAX_LETTERS];
    //int sensorId;
    //int time;      ya esta en dateFormatted
    int counts;
} TReading;

typedef struct sensor{
    int id;
    char * name;
    TReading maxCount;
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
    TSensor * sensorsVec;  // vector dinámico de estructuras TSensor, cada sensor (con cierto Id) esta en el indice [sensor Id-1]
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

static void freeName(TSensor * sensors, size_t dim){
    for(int i=0; i<dim; i++){
        if(sensors[i].id !=-1){
            free(sensors[i].name);
        }
    }
    free(sensors);
}//TESTED

void freePeatones(peatonesADT pea){
    freeRecList(pea->first);
    freeName(pea->sensorsVec, pea->sensorsSize);
    free(pea);
}//TESTED

void putSensor(peatonesADT pea, int id, char * name){
    if(id > pea->sensorsSize){
        // si no tengo espacio en el vector, agrando sensorVec.
        pea->sensorsVec = realloc(pea->sensorsVec, id * sizeof(TSensor));
        for(int i = pea->sensorsSize; i < id; i++)
            pea->sensorsVec[i].id = -1;
        pea->sensorsSize = id;
    }
    TSensor sensor;
    sensor.id = id;
    //sensor.sensorCounts ya esta en 0
    sensor.name = malloc(strlen(name) + 1); //hacer funcion copy auxiliar
    strcpy(sensor.name, name);
    //sensor.maxCount.counts ya esta en 0
    pea->sensorsVec[id-1]=sensor;
}//TESTED

int sensorExists(peatonesADT pea, int id){
    return (pea->sensorsSize >= id-1 && pea->sensorsVec[id-1].id!=-1);
}//TESTED

//si todavia no habia mediciones de ese anio, agrega un nodo a la lista y lo completa
//si ya estaba suma al count
static TYearList addYear(TYearList list, int year, int count){
    if (list==NULL || list->year < year){
        TYearList aux = malloc(sizeof(TNodeYear));
        aux->year = year;
        aux->yearCount = count;
        aux->tail = list;
        return aux;
    }
    if (list->year > year){
        list->tail = addYear(list->tail, year, count);
        return list;
    }
    list->yearCount += count;
    return list;
}


static int weekDayToNum (char day[10]){
    char * days[CANT_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for(int i=0; i<CANT_DAYS; i++){
        if(strcmp(day, days[i])==0)return i;
    }
    return -1;
}

//el vector FromTo tiene en el indice 0 desde que anio y en el indice 1 hasta que anio se debe considerar para el maxCount,
//y 0 si no se especifico un rango o un hasta(ambos espacios son 0 o FromTo[1]==0, respectivamente)
int addReading(peatonesADT pea, int sensorId, const int date[DATE_FIELDS], const char * day, int counts, const int FromTo[2]){
    if (!(sensorExists(pea, sensorId)))return 0;

    TSensor sensor = pea->sensorsVec[sensorId-1];
    sensor.sensorCounts += counts;
    //si esta entre los anios del rango provisto y counts es mayor que el counts que habia en maxCounts, lo modifica
    if (FromTo[0]==0 ||((date[YEAR]>=FromTo[0] && (date[YEAR]<=FromTo[1]))||FromTo[1]==0)){
        if (sensor.maxCount.counts < counts){
            sensor.maxCount.counts = counts;
            sensor.maxCount.dateFormatted[DAY]=date[DAY];
            sensor.maxCount.dateFormatted[MONTH]= date[MONTH];
            sensor.maxCount.dateFormatted[YEAR]=date[YEAR];
            sensor.maxCount.dateFormatted[HOUR]=date[HOUR];
        }
    }
    pea->sensorsVec[sensorId-1]=sensor; //paso al ADT lo que modifique

    pea->first = addYear(pea->first, date[YEAR], counts);

    int weekDay = ;
    if (date[HOUR] >= 6 && date[HOUR] < 18){
        pea->dayVec[weekDay].daylightCount += counts;
    } else
        pea->dayVec[weekDay].nightCount += counts;

    return 1;
}

char* getNameById(peatonesADT pea, int sensorID){
    if(  !sensorExists(pea, sensorID) ) return NULL;
    return pea->sensorsVec[sensorID-1].name;
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
        if (pea->sensorsVec[i].id != -1){
            sensorIDs[j++] = i+1;//se hace el fixeo basandose en que el valor de los ids está "corrido" un lugar respecto de los indices del vec (id=1 -> pos=0)
        }
    }
    *dim = j;
    return sensorIDs;
}//TESTED

long int getSensorCount(peatonesADT pea, int sensorID){
    return pea->sensorsVec[sensorID-1].sensorCounts;
}//TESTED

int hasNextYear(peatonesADT pea){
    return pea->next != NULL;
} //NEEDS addReading BEFORE TESTING

static int compareInt(int num1, int num2){
  return num1 - num2;
}

/*
 static int getYearRec(TYearList list, int year){
    if(list==NULL || compareInt(list->year, year) < 0) return -1;
    if(list->year == year){
        return year;
    }
    return getYearRec(list->tail, year);
} //NEEDS addReading BEFORE TESTING

long int getYearCount(peatonesADT pea, int year){
    return getYearRec(pea->first, year);

}
*/


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
    if(!sensorExists(pea, id) || pea->sensorsVec[id-1].maxCount.counts == 0) return 0; // chequeo si existe el sensor o si no hay una medida máxima
    *maxCount = pea->sensorsVec[id-1].maxCount.counts;
    *name = pea->sensorsVec[id-1].name;
    for (int field = 0; field < DATE_FIELDS ; field++) {
        date[field] = pea->sensorsVec[id-1].maxCount.dateFormatted[field];
    }
    return 1;
} //NEEDS addReading BEFORE TESTING





