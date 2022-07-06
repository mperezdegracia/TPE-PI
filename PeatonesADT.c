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
  // int * sensorIdsSorted;
    size_t cantSensores;
    TSensor * sensorsVec;  // vector dinámico de estructuras TSensor, cada sensor (con cierto Id) esta en el indice [sensor Id-1]
    size_t sensorsSize;

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

int putSensor(peatonesADT pea, int id, char * name){
    if(id > pea->sensorsSize){
        // si no tengo espacio en el vector, agrando sensorVec.
        pea->sensorsVec = realloc(pea->sensorsVec, id * sizeof(TSensor));
        if(errno == ENOMEM) return ENOMEM;

        for(int i = pea->sensorsSize; i < id; i++) {
            pea->sensorsVec[i].id = -1;
            pea->sensorsVec[i].sensorCounts=0;
        }
        pea->sensorsSize = id;
    }
    TSensor sensor;
    sensor.name = malloc(strlen(name) + 1); //hacer funcion copy auxiliar
    if(errno == ENOMEM) return ENOMEM;
    sensor.id = id;
    //sensor.sensorCounts ya esta en
    strcpy(sensor.name, name);
    //sensor.maxCount.counts ya esta en 0
    pea->cantSensores++;
    //pea->sensorIdsSorted = realloc(pea->sensorIdsSorted, pea->cantSensores);
    //pea->sensorIdsSorted[ pea->cantSensores -1 ] = id;
    pea->sensorsVec[id-1]=sensor;
    return OK;
}//TESTED

int sensorExists(peatonesADT pea, int id){
    return (pea->sensorsSize > id-1 && pea->sensorsVec[id-1].id!=-1);
}//TESTED

//si todavia no habia mediciones de ese anio, agrega un nodo a la lista y lo completa
//si ya estaba suma al count
static TYearList addYearRec(TYearList list, int year, int count){
    if (list==NULL || list->year < year){
        TYearList aux = calloc(1, sizeof(TNodeYear));

        if(aux == NULL || errno == ENOMEM){
            errno = ENOMEM;
            return list;
        }

        aux->year = year;
        aux->yearCount = count;
        aux->tail = list;
        return aux;
    }
    if (list->year > year){
        list->tail = addYearRec(list->tail, year, count);
        return list;
    }
    list->yearCount += count;
    return list;
}
int addYear(peatonesADT pea, int year, int count){
    pea->first = addYearRec(pea->first, year, count);
    if(errno==ENOMEM){
        return errno;
    }
    return OK;
}

static int weekDayToNum (const char *day){
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
    if (FromTo[0]==0 || (date[YEAR]>=FromTo[0] && ((date[YEAR]<=FromTo[1]) || FromTo[1]==0))){
        if (sensor.maxCount.counts < counts){
            sensor.maxCount.counts = counts;
            for(int field = DAY; field< DATE_FIELDS; field++) sensor.maxCount.dateFormatted[field]=date[field];
            /*sensor.maxCount.dateFormatted[MONTH]= date[MONTH];
            sensor.maxCount.dateFormatted[YEAR]=date[YEAR];
            sensor.maxCount.dateFormatted[HOUR]=date[HOUR];*/
        }
    }
    pea->sensorsVec[sensorId-1]=sensor; //paso al ADT lo que modifique

    int status = addYear(pea, date[YEAR], counts);
    if(status != OK){
        return status; // error
    }

    int weekDay = weekDayToNum(day);
    if (date[HOUR] >= 6 && date[HOUR] < 18){
        pea->dayVec[weekDay].daylightCount += counts;
    } else
        pea->dayVec[weekDay].nightCount += counts;

    return OK;
}

char* getNameById(peatonesADT pea, int sensorID){
    if(  !sensorExists(pea, sensorID) ) return NULL;
    return pea->sensorsVec[sensorID-1].name;
} //TESTED

long int getDailyCount(peatonesADT pea, int day, char option){
    //partiendo de que los parametros day y option son válidos se devuelve el campo que almacena los resultados totales de peatones por día y según el horario
    if(day < 0 || day > 6) return EDAY;
    if (option){
        return pea->dayVec[(int)day].nightCount;
    }
    return pea->dayVec[(int)day].daylightCount;
} //TESTED

int getCantSensores(peatonesADT pea){
    return pea->cantSensores;
}
int getSensorIDs(peatonesADT pea, int sensorIds[]){
    int i, j;
    for ( i = 0, j = 0; i < pea->sensorsSize; i++){
        if (pea->sensorsVec[i].id != -1){
            sensorIds[j++] = i+1;//se hace el fixeo basandose en que el valor de los ids está "corrido" un lugar respecto de los indices del vec (id=1 -> pos=0)
        }
    }
    return OK;
}//TESTED

long int getSensorCount(peatonesADT pea, int sensorID){
    if(!sensorExists(pea, sensorID)) return EID;
    return pea->sensorsVec[sensorID-1].sensorCounts;
}//TESTED

int hasNextYear(peatonesADT pea){
    return pea->next != NULL;
} //TESTED

void toBeginYear(peatonesADT pea){
    pea->next = pea->first;
} //TESTED

int nextYear(peatonesADT pea){
    if( !hasNextYear(pea)) return ENONEXT;
    pea->next =  pea->next->tail;
    return OK;
}//TESTED

int getYear(peatonesADT pea){
    return pea->next->year;
}
long int getCount(peatonesADT pea){
    return pea->next->yearCount;
}
int hasMaxReading(peatonesADT pea, int id){
    if(!sensorExists(pea, id)) return EID;
    return pea->sensorsVec[id-1].maxCount.counts!=0;
}
int getMaxReadingById(peatonesADT pea, int id, int * maxCount, char ** name, int date[DATE_FIELDS]){
    if(!sensorExists(pea, id)) return EID; // chequeo si existe el sensor o si no hay una medida máxima
    *maxCount = pea->sensorsVec[id-1].maxCount.counts;
    *name = pea->sensorsVec[id-1].name;
    for (int field = 0; field < DATE_FIELDS ; field++) {
        date[field] = pea->sensorsVec[id-1].maxCount.dateFormatted[field];
    }
    return OK;
} //TESTED

int getMaxCount(peatonesADT pea, int id){
    return pea->sensorsVec[id-1].maxCount.counts;
}
void sortMax(peatonesADT pea){
    qsort(pea->sensorsVec, pea->sensorsSize, sizeof(pea->sensorsVec[0]), compareMax);
}
void sortTotal(peatonesADT pea){
    qsort(pea->sensorsVec, pea->sensorsSize, sizeof(pea->sensorsVec[0]), compareTotal);
}
void eliminaCeros(peatonesADT pea){
    int i, j;
    for( i=0, j=0; j < pea->cantSensores && i < pea->sensorsSize; i++){
        if(pea->sensorsVec[i].id != -1){
            pea->sensorsVec[j++] = pea->sensorsVec[i];
        }
    }
    pea->sensorsSize =pea->cantSensores;
    pea->sensorsVec = realloc(pea->sensorsVec, sizeof(pea->sensorsVec[0]) * pea->sensorsSize+1);
}
int compareMax (const void * a, const void * b) {
    TSensor *r1 = (TSensor *) a;
    TSensor *r2 = (TSensor *) b;
    if( r2->maxCount.counts == r1->maxCount.counts){
        return strcmp(r1->name, r2->name);
    }
    return r2->maxCount.counts - r1->maxCount.counts;
    //return r1->maxCount.counts - r2->maxCount.counts;
}
int compareTotal (const void * a, const void * b) {
    TSensor *r1 = (TSensor *) a;
    TSensor *r2 = (TSensor *) b;
    if( r1->sensorCounts == r2->sensorCounts){
        return strcmp(r1->name, r2->name);
    }
    return (int)(r2->sensorCounts - r1->sensorCounts);
}




