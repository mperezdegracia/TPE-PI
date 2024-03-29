#include "PeatonesADT.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * en la esturctura peatonesCDT:
 *
 * es necesario que los anios esten ordenados para que despues sea mas eficiente el query 2, y se desconoce
 * el anio a partir del cual se comienzan a tomar mediciones, por lo que consideramos que convenia una lista
 *
 * la cantidad de dias de semana es un valor fijo, por lo que lo mejor es usar un vector estatico
 *
 * para los sensores la eleccion fue mas difícil porque se sabe muy poco acerca de los valores que puede tomar
 * el id. Decidimos priorizar la eficiencia en la carga de datos utilizando un vector dinamico, para que el acceso
 * al completar sea de orden 1. Además en los Queries 1 y 4 se pide listar en base a valores que solo se tienen una
 * vez terminada la carga del dataset, por lo que nos pareció más conveniente ordenar utilizando un vector
 */

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
    unsigned int amountSensors;   // cantidad de sensores activos que hay en el vector
    TSensor * sensorsVec;  // vector dinámico de estructuras TSensor, cada sensor (con cierto Id) esta en el indice [sensor Id-1]
    unsigned int sensorsSize;   // dimension del vector de sensores

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
    if(list==NULL){
        return;
    }
    freeRecList(list->tail);
    free(list);
}

//funcion auxiliar que libera el nombre en las estructuras Tsensor del vector
static void freeName(TSensor * sensors, size_t dim){
    for(int i=0; i<dim; i++){
        if(sensors[i].id != E_NOT_FOUND){
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
            pea->sensorsVec[i].id = E_NOT_FOUND;
            pea->sensorsVec[i].sensorCounts=0;
        }
        pea->sensorsSize = id;
    }
    else if (pea->sensorsVec[id-1].id != E_NOT_FOUND){   // ya existia ese sensor
        return E_ID;
    }
    TSensor sensor;
    //maxCount y sensorsCount ya estan en 0
    sensor.name = malloc(strlen(name) + 1);
    if(errno == ENOMEM) {
        return ENOMEM;
    }
    sensor.id = id;
    strcpy(sensor.name, name);
    pea->amountSensors++;
    pea->sensorsVec[id-1]=sensor;
    return OK;
}

int sensorExists(peatonesADT pea, int id){
    return (pea->sensorsSize > id-1 && pea->sensorsVec[id-1].id!= E_NOT_FOUND);
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

static int addYear(peatonesADT pea, int year, int count){
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
    return E_NOT_FOUND;
}

int addReading(peatonesADT pea, int id, const int date[DATE_FIELDS], const char * day, int counts, const int FromTo[2]){
    if (!(sensorExists(pea, id))){
        return E_ID;
    }

    TSensor sensor = pea->sensorsVec[id-1];
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
    pea->sensorsVec[id-1] = sensor; //paso al ADT lo que modifique

    int status = addYear(pea, date[YEAR], counts);
    if(status != OK){
        return status; // ENOMEM
    }

    int weekDay = weekDayToNum(day);
    if (date[HOUR] >= 6 && date[HOUR] < 18){
        pea->dayVec[weekDay].daylightCount += counts;
    } else {
        pea->dayVec[weekDay].nightCount += counts;
    }
    return OK;
}

char * getNameById(peatonesADT pea, int id){
    if(!sensorExists(pea, id)){
        return NULL;
    }
    return pea->sensorsVec[id-1].name;
}

long int getDailyCount(peatonesADT pea, int day, char option){
    //partiendo de que los parametros day y option son válidos se devuelve el campo que almacena los resultados totales de peatones por día y según el horario
    if(day < 0 || day > 6) {
        return E_DAY;
    }
    if (option){
        return pea->dayVec[(int)day].nightCount;
    }
    return pea->dayVec[(int)day].daylightCount;
}

unsigned int getSensorsAmount(peatonesADT pea){
    return pea->amountSensors;
}

long int getSensorCount(peatonesADT pea, int id){
    if(!sensorExists(pea, id)){
        return E_ID;
    }
    return pea->sensorsVec[id-1].sensorCounts;
}

int hasNextYear(peatonesADT pea){
    return pea->next != NULL;
}

void toBeginYear(peatonesADT pea){
    pea->next = pea->first;
}

int nextYear(peatonesADT pea){
    if( !hasNextYear(pea)){
        return E_NO_NEXT;
    }
    pea->next =  pea->next->tail;
    return OK;
}

int getYear(peatonesADT pea){
    return pea->next->year;
}

long int getYearCount(peatonesADT pea){
    return pea->next->yearCount;
}

int getDate(peatonesADT pea, int id, int date[DATE_FIELDS]){
    if(!sensorExists(pea, id)){
        return E_ID; // chequeo si existe el sensor o si no hay una medida máxima
    }
    for (int field = DAY; field < DATE_FIELDS ; field++) {
        date[field] = pea->sensorsVec[id-1].maxCount.dateFormatted[field];
    }
    return OK;
}

int getMaxCount(peatonesADT pea, int id){
    return pea->sensorsVec[id-1].maxCount.counts;
}

//Teniendo en cuenta los parametros que toma qsort, utilizado en sortMax y sortTotal
// las funciones auxiliares para comparar deben tener el siguiente prototipo:
// int (*comparator)(const void* p1,const void* p2)
// de modo que su valor de retorno indicará el orden en el que se ubiquen en el vector siendo:
// <0  si el elemento apuntado por p1 va antes del apuntado por p2
//  0  si el elemento apuntado por p1 es equivalente al apuntado por p2
// >0  si el elemento apuntado por p1 va despues del apuntado por p2
// A continuación las definimos para cada caso pedido:
static int compareMax (const void * a, const void * b) {
    TSensor *r1 = (TSensor *) a;
    TSensor *r2 = (TSensor *) b;
    if( r2->maxCount.counts == r1->maxCount.counts){
        return strcmp(r1->name, r2->name);
    }
    return r2->maxCount.counts - r1->maxCount.counts;
}

static int compareTotal (const void * a, const void * b) {
    TSensor *r1 = (TSensor *) a;
    TSensor *r2 = (TSensor *) b;
    if( r1->sensorCounts == r2->sensorCounts){
        return strcmp(r1->name, r2->name);
    }
    //no se devuelve la resta para evitar errores de casteo de long int a int
    if(r2->sensorCounts > r1->sensorCounts) {
        return 1;
    }
    return E_NOT_FOUND;
}

void sortMax(peatonesADT pea){
    qsort(pea->sensorsVec, pea->sensorsSize, sizeof(pea->sensorsVec[0]), compareMax);
}

void sortTotal(peatonesADT pea){
    qsort(pea->sensorsVec, pea->sensorsSize, sizeof(pea->sensorsVec[0]), compareTotal);
}

void deleteGaps(peatonesADT pea){
    int i, j;
    for( i=0, j=0; j < pea->amountSensors && i < pea->sensorsSize; i++){
        if(pea->sensorsVec[i].id != E_NOT_FOUND){
            pea->sensorsVec[j++] = pea->sensorsVec[i];
        }
    }
    pea->sensorsSize = pea->amountSensors;
    pea->sensorsVec = realloc(pea->sensorsVec, sizeof(pea->sensorsVec[0]) * pea->sensorsSize+1);
}

