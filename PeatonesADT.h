//
// Created by MateoPérezdeGracia on 6/29/2022.
//
/*Query 1: Total de Peatones por Sensor

Donde cada línea de la salida contenga, separados por “;” el nombre del sensor y la cantidad total de peatones registrados por el sensor.
La información debe listarse ordenada en forma descendente por cantidad total de peatones y a igualdad de peatones desempatar alfabéticamente por nombre de sensor.

sensor;counts
Town Hall (West);185243492
Flinders Street Station Underpass;163910450
Bourke Street Mall (South);109987361
Melbourne Central;109987361
Princes Bridge;91140173
...

 *
 */
/*Query 2: Total de Peatones por Año

Donde cada línea de la salida contenga, separados por “;” el año y la cantidad total de peatones registrados por los sensores activos en ese año.
La información debe listarse ordenada en forma descendente por año.

year;counts
2022;73286121
2021;113913422
2020;96846536
2019;210333736
2018;199956553
...

*/
/*Query 3: Total de peatones por día de la semana y por período del día

Donde cada línea de la salida contenga, separados por “;” el día de la semana, la cantidad total de peatones registrados por los sensores activos
 durante un rango diurno (entre las 6 horas inclusive y las 18 horas no inclusive), la cantidad total de peatones registrados por los sensores activos durante un
 rango nocturno (entre las 0 horas inclusive hasta las 6 horas y entre las 18 horas inclusive hasta las 0 horas) y la suma de ambos valores.
La información debe listarse ordenada en forma cronológica por día de la semana donde el Lunes es el primer día de la semana.

day;day_counts;night_counts;total_counts
Monday;182500261;54267406;236767767
Tuesday;189118525;58010097;247128622
Wednesday;193753100;63417800;257170900
Thursday;194964083;68210933;263175016
Friday;99229378;2369605;101598983
Saturday;156200000;90053472;246253472
Sunday;144543185;63016000;207559185

*/
/*Query 4: La medición máxima de cada sensor

Donde cada línea de la salida contenga, separados por “;” el nombre del sensor, la cantidad total de peatones registrados en la medición máxima histórica del sensor,
 la hora de esa medición y el día, mes y año de esa medición en formato DD/MM/YYYY.
La información debe listarse ordenada en forma descendente por cantidad total de peat a igualdad de peatones desempatar alfabéticamente por nombre de sensor.
ones registrados en la medición máxima histórica y

sensor;max_counts;hour;date
Bourke St Bridge;14121;23;14/11/2019
Birrarung Marr;13291;22;24/8/2019
Southbank;7483;22;30/12/2019
St Kilda Rd-Alexandra Gardens;7483;20;12/3/2022
New Quay;6948;10;1/4/2012
...

*/

#ifndef TPE_PEATONESADT_H
#define TPE_PEATONESADT_H

typedef struct peatonesCDT* peatonesADT;
typedef enum dateType { HOUR=0, DAY, MONTH, YEAR, DATE_FIELDS } dateType;
typedef enum daysType { MONDAY=0, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY, CANT_DAYS } daysType;


/*
 * Crea y devuelve un TAD vacío
 */
peatonesADT newPeatones(void);
/*
 * Libera el TAD
 */
void freePeatones(peatonesADT peat);
/*
 * Agrega un sensor y devuelve un 1 si lo creó y un 0 si ya existe.
 */


/*
 * arranca el programa main
 * crea tad vacio
 * abrir archivo de sensores
 * carga todos los sensores activos
 * abro el archivo de mediciones
 * addReading a todos las mediciones
 *
 *
 *
 *
 *
 *
 *
 * */
int putSensor(peatonesADT pea, int id, char * name);    // puse put xq uso vectores
/*
 * incrementa los respectivos counts, si la medición es máxima, entonces actualizo el maxCount del respectivo sensor
 */
int addReading(peatonesADT pea, int year, char * month, int mDate, char * day, int sensorId, int time, int counts);   // puse add xq uso listas


/*
 * devuelve 1 si el sensor está en la lista de sensores activos y 0 de lo contrario
 */
int sensorExists(peatonesADT pea, int id);


//QUERY 1
/*
 * devuelve la cantidad de peatones que leyó un sensor
 */
long int getSensorCount(peatonesADT pea, int sensorID);
/*
 * devuelve el nombre del sensor
 */
char* getNameById(peatonesADT pea, int sensorID);


/*
 * devuelve un vector con los ID de los sensores activos y la dimension de ese vector en un parametro de salida
 * los IDs despues necesitan para pasarle el parametro a getSensorCount
 */
int * getSensorIDs(peatonesADT pea, int * dim);

//QUERIES 2
/*
 * devuelve la cantidad de peatones de un determinado año.
 */
long int getYearCount(peatonesADT pea, int year);

/*
 * setea el iterador en el primer año de la lista
 */
void toBeginYear(peatonesADT pea);

/*
 * obtiene el siguiente año en la lista
 */
int nextYear(peatonesADT pea);

/*
 * determina si hay un siguiente año en la lista
 * devuelve 0 si se llegó al final de la misma y 1 en el caso contrario
 */
int hasNextYear(peatonesADT pea);

//QUERIES 3
/*
 *  devuelve la cantidad de peatones en horario diurno/nocturno del día especificado
 *  se pasa un BOOL donde 0 es para daylight y 1 es para night
 */
long int getDailyCount(peatonesADT pea, char day, char option);


//QUERIES 4

/*
 * Devuelve 1 si tuvo éxito o 0 si hubo error. deja los campos en los parámetros de salida de un sensor con SensorId=id
 *
 */
int getMaxReadingById(peatonesADT pea, int id, int * maxCount, char ** name, int date[DATE_FIELDS]); // hay que formatear el date


//getDateFormatted
#endif //TPE_PEATONESADT_H