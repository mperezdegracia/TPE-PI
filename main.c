#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


#define CANT_QUERYS 4
#define BUFF_SIZE 512
#define FALSE 0
#define DATA_FILES 2
#define TRUE !FALSE
#define DAYLIGHT 0
#define NIGHT !DAYLIGHT
#define DELIM_FIELD ";"
#define UPDATE strtok(NULL, DELIM_FIELD) // macro para pasar al siguiente token

enum {FROM=0, TO, FROM_TO};
enum {FILENAME=0, FILE1, FILE2, RANGE1, RANGE2};

int fillAdt(peatonesADT tad, FILE* dataSensors, FILE* dataReadings, int * yearRange);

//  Imprime un mensaje de error y aborta el programa
void errorExit (int errValue, char * errMessage, char * arg);

//  Imprime un mensaje de error, libera los recursos usados, cierra todos los archivos y aborta el programa
void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount, peatonesADT tad);

//  Cierra todos los archivos utilizados
void closeAllFiles (FILE * files[], size_t fileCount);

//Funcion que imprime en cada archivo la primera linea, que indica que es lo que representa cada columna del archivo
void printQueryTitles(FILE * query1, FILE * query2, FILE * query3, FILE * query4);

// Las funciones loadQuery 1, 2, 3 y 4 cargan sobre el archivo recibido queryX.csv con los resultados de cada consulta
// Si hubo algun problema en la escritura del archivo devuelven E_FILE
int loadQuery1 (peatonesADT tad, FILE* query1);

int loadQuery2 (peatonesADT tad, FILE* query2);

int loadQuery3 (peatonesADT tad, FILE* query3);

int loadQuery4 (peatonesADT tad, FILE* query4);

//  Recibe un string y devuelve 1 si es un numero y 0 en caso contrario
int stringIsNumber (const char * num);

//  Funcion auxiliar que devuelve el numero del mes
int monthToNum (char * month);

//  Funcion auxiliar que devuelve el día de la semana en string de acuerdo a su numeración
char * numToDay (int num);


int main(int argc, char * argv[]){

    // Validacion de parametros (EINVAL: argumento invalido)
    int yearRange[FROM_TO]={0, 0};
    int validArg = TRUE;
    switch (argc) {
        case 3:
            break;
        case 4:
            if (!stringIsNumber(argv[RANGE1])){
                validArg = FALSE;
                yearRange[FROM] = -1;
            } else {
                yearRange[FROM] = atoi(argv[RANGE1]);
            }
            break;
        case 5:
            if (!stringIsNumber(argv[RANGE1])||!stringIsNumber(argv[RANGE2])){
                validArg = FALSE;
                yearRange[FROM] = -1;
            } else {
                yearRange[FROM] = atoi(argv[RANGE1]);
                yearRange[TO] = atoi(argv[RANGE2]);
            }
            if(yearRange[1] != 0 && yearRange[TO] < yearRange[FROM])
                validArg = FALSE;
        default:
            errorExit(EINVAL, "Cantidad invalida de argumentos", argv[FILENAME]);
            break;
    }

    // **********DECLARACION DE ARCHIVOS **********

    FILE * dataSensors = fopen(argv[FILE2], "r");
    FILE * dataReadings= fopen(argv[FILE1], "r");
    FILE * query1 = fopen("query1.csv", "w");
    FILE * query2 = fopen("query2.csv", "w");
    FILE * query3 = fopen("query3.csv", "w");
    FILE * query4 = fopen("query4.csv", "w");
    FILE * files[] = {dataSensors, dataReadings, query1, query2, query3, query4};
    size_t fileCount = CANT_QUERYS + DATA_FILES;


    // Revisamos que no hubo ningun error al abrir los archivos
    // ENOENT: no existe dicho archivo.
    // ENOMEM: memoria insuficiente

    for (size_t i = 0; i < fileCount; i++) {
        if (files[i] == NULL) {
            closeAllFiles(files, fileCount);
            if(i < DATA_FILES){
                errorExit(ENOENT, "Error al abrir el dataset", argv[FILENAME]);
            }
            errorExit(ENOMEM, "No se pudo abrir uno de los archivos", argv[FILENAME]);
        }
    }

    //  NUEVO TAD
    peatonesADT  tad = newPeatones();

    if (tad == NULL || errno == ENOMEM) { //  SI NO SE PUDO CREAR EL TAD
        closeAllFiles(files, fileCount);
        errorExit(ENOMEM, "ERROR : No hay memoria suficiente en el heap", argv[FILENAME]);
    }


    // LLENAMOS EL TAD CON LOS DATOS DE LOS DATASETS
    int status = fillAdt(tad, dataSensors, dataReadings, yearRange);

    if(status == E_FILE){
        closeExit(files, EINVAL, "ERROR : durante la carga de datos, el archivo ingresado esta vacio", argv[FILENAME], fileCount, tad);
    }
    if(status == ENOMEM){
        errorExit(ENOMEM, "ERROR : No hay memoria suficiente en el heap", argv[FILENAME]);
    }

    deleteGaps(tad);

    // IMPRIMIMOS EN LOS ARCHIVOS QUERY LA PRIMERA LINEA, QUE INDICA QUE REPRESENTA LA INFORMACION DE ESE ARCHIVO
    printQueryTitles(query1, query2, query3, query4);

    // EJECUTAMOS QUERIES

    // CARGA DE DATOS A LOS ARCHIVOS QUERY 1, 2, 3 y 4
    if (loadQuery1 (tad, query1) < 0)
        closeExit(files, E_FILE, "Hubo un error en la carga del query1", argv[0], fileCount, tad);

    if (loadQuery2 (tad, query2) < 0)
        closeExit(files, E_FILE, "Hubo un error en la carga del query2", argv[0], fileCount, tad);

    if (loadQuery3 (tad, query3) < 0)
        closeExit(files, E_FILE, "Hubo un error en la carga del query3", argv[0], fileCount, tad);

    if (!validArg){
        closeExit(files, EINVAL, "los parametros del rango de años son incorrectos", argv[0], fileCount, tad);
    } else if (loadQuery4 (tad, query4) < 0) {
        closeExit(files, E_FILE, "Hubo un error en la carga del query4", argv[0], fileCount, tad);
    }

    closeAllFiles(files, fileCount);
    freePeatones(tad);
}
//**********************************************************************************************************************

int fillAdt(peatonesADT tad, FILE* dataSensors, FILE* dataReadings, int * yearRange){

    //  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    int id;
    char buff[BUFF_SIZE], * token, * name; // en buff se van a ir llegando las lineas del .csv. BUFF_SIZE es un tamaño arbitrario

    // Si la primer linea del archivo dataSensors esta vacia, retorna un mensaje de error y aborta el programa
    if (fgets(buff, BUFF_SIZE, dataSensors) == NULL) {
        return E_FILE;
    }

    while (fgets(buff, BUFF_SIZE, dataSensors) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
        id = atoi(token);
        if(!sensorExists(tad, id)) { // si el id no es duplicado
            token = UPDATE;
            name = token;
            token = UPDATE;
            if (token[0] == 'A'){

                putSensor(tad, id, name); // creo los sensores
            }
        }
    }

    //**********DATA READINGS**********

    // VARIABLES QUE LLENAMOS CON DATA_READINGS
    char * Wday;
    int sensorId, counts, status;
    int dateFormatted[DATE_FIELDS];
    if (fgets(buff, BUFF_SIZE, dataReadings) == NULL) {
        return E_FILE;
    }

    while (fgets(buff, BUFF_SIZE, dataReadings) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
        dateFormatted[YEAR] = atoi(token);
        token = UPDATE;
        dateFormatted[MONTH] = monthToNum(token);
        token = UPDATE;
        dateFormatted[DAY] = atoi(token);
        token = UPDATE;
        Wday = token;
        token = UPDATE;
        sensorId = atoi(token);
        token = UPDATE;
        dateFormatted[HOUR] = atoi(token);
        token = UPDATE;
        counts = atoi(token);
        token = UPDATE; //token vale NULL

        if (sensorExists(tad, sensorId)) {     // solo se agrega la informacion si el sensor existe y es valido
            status = addReading(tad, sensorId, dateFormatted, Wday, counts, yearRange); // creo los sensores
            if(status == ENOMEM){
                return ENOMEM;
            }
        }
    }
    return OK;
}

void printQueryTitles(FILE * query1, FILE * query2, FILE * query3, FILE * query4){
    fprintf(query1, "sensor;counts\n");
    fprintf(query2, "year;counts\n");
    fprintf(query3, "day;day_counts;night_counts;total_counts\n");
    fprintf(query4, "sensor;max_counts;hour;date\n");
}


int loadQuery1 (peatonesADT tad, FILE * query1){
    sortTotal(tad);
    long int count;
    char * name;
    for (int i=1; i <= getSensorsAmount(tad); i++) {
        count = getSensorCount(tad, i);
        name = getNameById(tad, i);
        if (name == NULL || count == E_ID) {
            return E_ID;
        }
        int res = fprintf(query1, "%s;%li\n", name, count);
        if (res < 0)
            return E_FILE;
    }
    return OK;
}

int loadQuery2 (peatonesADT tad, FILE * query2){
    toBeginYear(tad);
    int year;
    long int counts;
    while (hasNextYear(tad)){
        year = getYear(tad);
        counts = getCount(tad);
        int res = fprintf(query2, "%d;%li\n", year, counts);
        if (res < 0){
            return E_FILE;
        }
        nextYear(tad);
    }
    return OK;
}

int loadQuery3 (peatonesADT tad, FILE * query3){
    long int nightCount, dayCount;
    for (int day = MONDAY; day<CANT_DAYS; day++){
        nightCount = getDailyCount(tad, day, NIGHT);
        dayCount = getDailyCount(tad, day, DAYLIGHT);
        if ( nightCount < 0 || dayCount < 0){
            return E_DAY;
        }
        int res = fprintf(query3, "%s;%li;%li;%li\n", numToDay(day), dayCount, nightCount, dayCount+nightCount);
        if (res < 0)
            return E_FILE;
    }
    return OK;
}

int loadQuery4 (peatonesADT tad, FILE * query4){
    sortMax(tad);
    int count;
    char* name;
    int dateFormatted[DATE_FIELDS];
    for (int i=1; i <= getSensorsAmount(tad); i++){
        count = getMaxCount(tad, i);
        if(count) { //si el sensor tiene mediciones (si no tiene mediciones no tiene maximo, asi que no imprime nada)
            name = getNameById(tad, i);
            getDate(tad, i, dateFormatted);
            int res = fprintf(query4, "%s;%d;%d;%d/%d/%d\n", name, count, dateFormatted[3], dateFormatted[0],
                              dateFormatted[1], dateFormatted[2]);
            if (res < 0)
                return E_FILE;
        }
    }
    return OK;
}

int stringIsNumber (const char * num){
    while(*num != 0){
        if (!isdigit(*num))
            return 0;
        num++;
    }
    return 1;
}

int monthToNum (char * month){
    char * months[CANT_MONTH] = {"January", "February", "March","April","May", "June", "July",
                                 "August", "September", "October", "November", "December"};
    for (int i=0; i<CANT_MONTH; i++){
        if (strcmp(month, months[i])==0)
            return i+1;
    }
    return E_NOT_FOUND;
}
char * numToDay (int num){
    char * weekDay[CANT_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    if (num < 0 || num >= CANT_DAYS){
        return NULL;
    }
    return weekDay[num];
}

void errorExit (int errValue, char * errMessage, char * arg) {
    fprintf(stderr, "%s : %s\n", arg, errMessage);
    exit(errValue);
}

void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount, peatonesADT tad) {
    freePeatones(tad);
    closeAllFiles(files, fileCount);
    errorExit(errValue, errMessage, arg);
}

void closeAllFiles (FILE * files[], size_t fileCount) {
    for(size_t i = 0; i < fileCount; i++) {
        if (files[i] != NULL) {
            fclose(files[i]);
        }
    }
}


