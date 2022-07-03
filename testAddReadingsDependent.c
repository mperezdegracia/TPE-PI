//
// Created by UrielArias on 1/7/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "PeatonesADT.h"


/*
 * int addReading(peatonesADT pea, int year, char * month, int mDate, char * day, int sensorId, int time, int counts, const int FromTo[2]){
    TSensor sensor = pea->sensorsVec[sensorId-1];
    if (pea->sensorsSize < sensorId||sensor.id==-1) return 0;
    sensor.sensorCounts += counts;
    //si esta entre los anios del rango provisto y counts es mayor que el counts que habia en maxCounts, lo modifica
    if (FromTo[0]==0 ||((year>=FromTo[0] && (year<=FromTo[1]))||FromTo[1]==0)){
        if (sensor.maxCount.counts < counts){
            sensor.maxCount.dateFormatted[DAY]=mDate;
            sensor.maxCount.dateFormatted[MONTH]= monthToNum(month);
            sensor.maxCount.dateFormatted[YEAR]=year;
            sensor.maxCount.dateFormatted[HOUR]=time;
        }
    }
    pea->sensorsVec[sensorId-1]=sensor; //paso al ADT lo que modifique

    toBeginYear(pea);
    while (hasNextYear(pea) && pea->next->year < year){
        pea->next = pea->next->tail;
    }
    if (pea->next->year > year){
        pea->next = addYear(pea->next, year);
    }
    pea->next->yearCount += counts;

    int weekDay = weekDayToNum(day);
    if (time >= 6 && time < 18){
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
    putSensor(tad, 40, "Street 1");
    putSensor(tad, 140, "Street 34");
    addReading(tad, 2021, "June", 30, "Wednesday", 40, 12, 2389389, range);
    addReading(tad, 2015, "March", 21, "Thursday", 140, 22, 1761879, range);
    addReading(tad, 2012, "January", 21, "Friday", 146, 3, 17618793, range);
    addReading(tad, 2017, "March", 13, "Saturday", 140, 13, 7836829, range);
    addReading(tad, 2022, "December", 31, "Sunday", 40, 23, 9289726, range);
    toBeginYear(tad);
    int year;
    long int yearCount;
    nextYear(tad, &yearCount, &year);
    assert(year == 2022);
    assert(yearCount == 9289726);
    nextYear(tad, &yearCount,&year);
    assert(year == 2021);
    assert(yearCount == 2389389);
    nextYear(tad, &yearCount,&year);
    assert(year == 2017);
    assert(yearCount == 7836829);
    nextYear(tad, &yearCount,&year);
    assert(year == 2015);
    assert(yearCount == 1761879);

    assert(getDailyCount(tad, 2, 0) == 2389389);
    assert(getDailyCount(tad, 2, 1) == 0);
    assert(getDailyCount(tad, 3, 0) == 0);
    assert(getDailyCount(tad, 3, 1) == 1761879);
    assert(sensorExists(tad, 146) == 0);
    assert(getDailyCount(tad, 4, 1) == 0);

    int maxCount;
    char * sensorName;
    int date[DATE_FIELDS];
    getMaxReadingById(tad, 140, &maxCount, &sensorName, date);
    
}

