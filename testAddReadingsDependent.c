//
// Created by UrielArias on 1/7/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "PeatonesADT.h"


/*
 * int addReading(peatonesADT pea, int sensorId, const int date[DATE_FIELDS], const char * day, int counts, const int FromTo[2]){
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

    int weekDay = weekDayToNum(day);
    if (date[HOUR] >= 6 && date[HOUR] < 18){
        pea->dayVec[weekDay].daylightCount += counts;
    } else
        pea->dayVec[weekDay].nightCount += counts;

    return 1;
}
 long int getDailyCount(peatonesADT pea, char day, char option){
    //partiendo de que los parametros day y option son válidos se devuelve el campo que almacena los resultados totales de peatones por día y según el horario
    if (option){
        return pea->dayVec[(int)day].nightCount;
    }
    else
        return pea->dayVec[(int)day].daylightCount;
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
    if(!sensorExists(pea, id) || pea->sensorsVec[id-1].maxCount.counts == 0) return 0; // chequeo si existe el sensor o si no hay una medida máxima
    *maxCount = pea->sensorsVec[id-1].maxCount.counts;
    *name = pea->sensorsVec[id-1].name;
    for (int field = 0; field < DATE_FIELDS ; field++) {
        date[field] = pea->sensorsVec[id-1].maxCount.dateFormatted[field];
    }
    return 1;
} //NEEDS addReading BEFORE TESTING
 */


int main(void) {
    peatonesADT tad = newPeatones();
    int range[2] = {2015, 2022};

    int date1[DATE_FIELDS] = {30,5,2021,12};
    int date2[DATE_FIELDS] = {21,2,2015,22};
    int date3[DATE_FIELDS] = {21,0,2012,3};
    int date4[DATE_FIELDS] = {13,2,2015,22};
    int date5[DATE_FIELDS] = {31,11,2022,23};
    //incorporamos los sensores que deberían aparecer en el archivo de sensors usando la función ya testeada
    putSensor(tad, 40, "Street 1");
    putSensor(tad, 140, "Street 34");
    //agregamos mediciones utiles para chequear funciones
    addReading(tad, 40, date1, "Wednesday", 89389, range);
    addReading(tad, 140, date2, "Thursday", 61879, range);
    addReading(tad, 146, date3, "Friday", 1879, range);
    addReading(tad, 140, date4, "Saturday", 36829, range);
    addReading(tad, 40, date5, "Sunday", 89726, range);
    toBeginYear(tad);
    int year;
    long int yearCount;
    //verificamos que los años queden ordenados en orden descendente
    nextYear(tad, &yearCount, &year);
    assert(year == 2022);
    assert(yearCount == 89726);
    nextYear(tad, &yearCount,&year);
    assert(year == 2021);
    assert(yearCount == 89389);
    nextYear(tad, &yearCount,&year);
    assert(year == 2017);
    assert(yearCount == 36829);
    nextYear(tad, &yearCount,&year);
    assert(year == 2015);
    assert(yearCount == 61879);
    //verificamos que la medición no sea incorporada porque correspondería a un sensor no incluido en el archivo de sensores de importancia
    assert(hasNextYear(tad) == 0);
    //verificamos que se actualice el iterador
    toBeginYear(tad);
    nextYear(tad, &yearCount, &year);
    assert(year == 2022);
    assert(yearCount == 89726);
    //verificamos por día de la semana que se hayan actualizado los contadores en función del horario de las mediciones
    //daysType { MONDAY=0, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY, CANT_DAYS }
    assert(getDailyCount(tad, 2, 0) == 89389);
    assert(getDailyCount(tad, 2, 1) == 0);
    assert(getDailyCount(tad, 3, 0) == 0);
    assert(getDailyCount(tad, 3, 1) == 61879);
    assert(getDailyCount(tad, 4, 1) == 0);
    //verifico que el sensor no esta
    assert(sensorExists(tad, 146) == 0);
    int maxCount;
    char * sensorName;
    int date[DATE_FIELDS];
    int j;
    //verificamos que se guarde solo la medición máxima tomada por cada sensor
    getMaxReadingById(tad, 140, &maxCount, &sensorName, date);
    assert(maxCount == 61879);
    assert(strcmp(sensorName,"Street 34") == 0);
    for (j = 0; j < DATE_FIELDS; j++) {
        assert(date[j] == date2[j]);
    }
    getMaxReadingById(tad, 40, &maxCount, &sensorName, date);
    assert(maxCount ==  89726);
    assert(strcmp(sensorName,"Street 1") == 0);
    for (j = 0; j < DATE_FIELDS; j++) {
        assert(date[j] == date5[j]);
    }

}

