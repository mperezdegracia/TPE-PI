//
// Created by MateoPérezdeGracia on 6/29/2022.
//

#include "PeatonesADT.h"

#define DAY_MAX_LETTERS 10
#define MAX_DATE 20

typedef struct reading {

    //char * month ;    no se usan
    //int mDate;
    int year;
    char dateFormatted[MAX_DATE];  // el main tiene que pasar la feche formateada
    char day[DAY_MAX_LETTERS];
    int sensorId;
    int time;      //  [0,23]
    int counts;
} TReading;

typedef struct sensor{
    int id;
    char * name;
    //char status;
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
    struct NodeYear * next;
}TNodeYear;

typedef TNodeYear * TYearList;

typedef struct peatonesCDT {
    int sensorsSize;
    TSensor ** sensorsVec;  // vector dinámico de punteros TSensor
    TYearList first;  // list
    TYearList next;  // list
    TDay dayVec[CANT_DAYS];
} peatonesCDT;