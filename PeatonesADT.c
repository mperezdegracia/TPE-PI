#include "PeatonesADT.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//estructura para guardar la maxima medicion de cada sensor, y el dia, mes, anio y hora en el que se tomo esa medicion
typedef struct reading {
    int dateFormatted[DATE_FIELDS];  // vector de dim 4 con el dia, mes, anio y hora
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
    int year;   //anio de la medicion
    long int yearCount; //suma de la cantidad de peatones que se midieron en total ese anio
    struct NodeYear * tail; //puntero al siguiente nodo
}TNodeYear;

typedef TNodeYear * TYearList;

typedef struct peatonesCDT {
    size_t cantSensores;   // cantidad de sensores activos que hay en el vector
    TSensor * sensorsVec;  // vector dinámico de estructuras TSensor, cada sensor (con cierto Id) esta en el indice [sensor Id-1]
    size_t sensorsSize;   // dimension del vector de sensores

    TYearList first;  // el primer nodo de la lista que guarda la informacion segun el anio
    TYearList next;  // iterador para la lista

    TDay dayVec[CANT_DAYS]; // vector de dim 7 de los dias de la semana, en cada indice se guarda la cantidad total de peatones que midieron,
                            // historialmente, ese dia de la semana, separado en mediciones que fueron de dia y las que fueron de noche
} peatonesCDT;

peatonesADT newPeatones(void){
    return calloc(1, sizeof(peatonesCDT));
}

//funcion auxiliar para liberar recursivamente la lista
static void freeRecList(TYearList list){
    if(list==NULL) return;
    freeRecList(list->tail);
    free(list);
}

//funcion auxiliar que libera el nombre en las estructuras Tsensor del vector
static void freeName(TSensor * sensors, size_t dim){
    for(int i=0; i<dim; i++){
        if(sensors[i].id !=-1){
            free(sensors[i].name);
        }
    }
    free(sensors);
}

void freePeatones(peatonesADT pea){
    freeRecList(pea->first);
    freeName(pea->sensorsVec, pea->sensorsSize);
    free(pea);
}

int putSensor(peatonesADT pea, int id, char * name){
    if(id > pea->sensorsSize){
        // si no tengo espacio en el vector, agrando sensorVec.
        pea->sensorsVec = realloc(pea->sensorsVec, id * sizeof(TSensor));
        if(errno == ENOMEM) {
            return ENOMEM;
        }

        // completa los nuevos que agrego con id=-1 para reconocer que estan "vacios", y count en 0
        for(size_t i = pea->sensorsSize; i < id; i++) {
            pea->sensorsVec[i].id = -1;
            pea->sensorsVec[i].sensorCounts=0;
        }
        pea->sensorsSize = id;
    }
    else if (pea->sensorsVec[id-1].id != -1){   // ya existia ese sensor
        return EID;
    }
    TSensor sensor;
    //maxCount y sensorsCount ya estan en 0
    sensor.name = malloc(strlen(name) + 1);
    if(errno == ENOMEM) return ENOMEM;
    sensor.id = id;
    strcpy(sensor.name, name);
    pea->cantSensores++;
    pea->sensorsVec[id-1]=sensor;
    return OK;
}

int sensorExists(peatonesADT pea, int id){
    return (pea->sensorsSize > id-1 && pea->sensorsVec[id-1].id!=-1);
}

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

// funcion auxiliar que recibe el nombre de un dia de la semana y devuelve un numero que corresponda a ese dia,
// lunes siendo 0 y domingo siendo 6
static int weekDayToNum (const char *day){
    char * days[CANT_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for(int i=0; i<CANT_DAYS; i++){
        if(strcmp(day, days[i])==0)return i;
    }
    return -1;
}

int addReading(peatonesADT pea, int sensorId, const int date[DATE_FIELDS], const char * day, int counts, const int FromTo[2]){
    if (!(sensorExists(pea, sensorId)))return EID;

    TSensor sensor = pea->sensorsVec[sensorId-1];
    sensor.sensorCounts += counts;
    //si esta entre los anios del rango provisto y counts es mayor que el counts que habia en maxCounts, lo modifica
    if (FromTo[0]==0 || (date[YEAR]>=FromTo[0] && ((date[YEAR]<=FromTo[1]) || FromTo[1]==0))){
        if (sensor.maxCount.counts < counts){
            sensor.maxCount.counts = counts;
            for(int field = DAY; field < DATE_FIELDS; field++) {
                sensor.maxCount.dateFormatted[field] = date[field];
            }
        }
    }
    pea->sensorsVec[sensorId-1] = sensor; //paso al ADT lo que modifique

    int status = addYear(pea, date[YEAR], counts);
    if(status != OK){
        return status; // ENOMEM
    }

    int weekDay = weekDayToNum(day);
    if (date[HOUR] >= 6 && date[HOUR] < 18){
        pea->dayVec[weekDay].daylightCount += counts;
    } else
        pea->dayVec[weekDay].nightCount += counts;

    return OK;
}

char * getNameById(peatonesADT pea, int sensorID){
    if(  !sensorExists(pea, sensorID) ) return NULL;
    return pea->sensorsVec[sensorID-1].name;
}

long int getDailyCount(peatonesADT pea, int day, char option){
    //partiendo de que los parametros day y option son válidos se devuelve el campo que almacena los resultados totales de peatones por día y según el horario
    if(day < 0 || day > 6) return EDAY;
    if (option){
        return pea->dayVec[(int)day].nightCount;
    }
    return pea->dayVec[(int)day].daylightCount;
}

int getCantSensores(peatonesADT pea){
    return pea->cantSensores;
}

long int getSensorCount(peatonesADT pea, int sensorID){
    if(!sensorExists(pea, sensorID)) return EID;
    return pea->sensorsVec[sensorID-1].sensorCounts;
}

int hasNextYear(peatonesADT pea){
    return pea->next != NULL;
}

void toBeginYear(peatonesADT pea){
    pea->next = pea->first;
}

int nextYear(peatonesADT pea){
    if( !hasNextYear(pea)) return ENONEXT;
    pea->next =  pea->next->tail;
    return OK;
}

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

int getDate(peatonesADT pea, int id, int date[DATE_FIELDS]){
    if(!sensorExists(pea, id)) return EID; // chequeo si existe el sensor o si no hay una medida máxima
    for (int field = 0; field < DATE_FIELDS ; field++) {
        date[field] = pea->sensorsVec[id-1].maxCount.dateFormatted[field];
    }
    return OK;
}

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
    pea->sensorsSize = pea->cantSensores;
    pea->sensorsVec = realloc(pea->sensorsVec, sizeof(pea->sensorsVec[0]) * pea->sensorsSize+1);
}

int compareMax (const void * a, const void * b) {
    TSensor *r1 = (TSensor *) a;
    TSensor *r2 = (TSensor *) b;
    if( r2->maxCount.counts == r1->maxCount.counts){
        return strcmp(r1->name, r2->name);
    }
    return r2->maxCount.counts - r1->maxCount.counts;
}

int compareTotal (const void * a, const void * b) {
    TSensor *r1 = (TSensor *) a;
    TSensor *r2 = (TSensor *) b;
    if( r1->sensorCounts == r2->sensorCounts){
        return strcmp(r1->name, r2->name);
    }
    if(r2->sensorCounts > r1->sensorCounts) {
        return 1;
    }
    return -1;
}
