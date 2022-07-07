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
#define DELIM_FIELD ";"

enum {FROM=0, TO, FROM_TO};
enum {FILENAME, FILE1, FILE2, RANGE1, RANGE2, MAXARGS};
//  Imprime un mensaje de error y aborta el programa
void errorExit (int errValue, char * errMessage, char * arg);

int fillAdt(peatonesADT tad, FILE* dataSensors, FILE* dataReadings, int * yearRange);

//  Imprime un mensaje de error y cierra todos los archivos y aborta el programa

typedef enum readingFieldType {R_YEAR = 0, R_MONTH, MDATE, R_DAY, ID, TIME, COUNTS , CANT_FIELDS_READING} readingFieldType;

typedef enum sensorFieldType {SENSOR_ID = 0, NAME, STATUS, CANT_FIELDS_SENSOR} sensorFieldType;

//  Imprime un mensaje de error y aborta el programa
void errorExit (int errValue, char * errMessage, char * arg);

//  Imprime un mensaje de error, libera los recursos usados, cierra todos los archivos y aborta el programa
void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount, peatonesADT tad);

//  Cierra todos los archivos utilizados
void closeAllFiles (FILE * files[], size_t fileCount);

// Las funciones loadQuery 1, 2, 3 y 4 cargan sobre el archivo recibido queryX.csv con los resultados de cada consulta
// Si hubo algun problema en la escritura del archivo devuelven EFILE, EID o EDAY segun el error corespondiente
int loadQuery1 (peatonesADT tad, FILE* query1);

int loadQuery2 (peatonesADT tad, FILE* query2);

int loadQuery3 (peatonesADT tad, FILE* query3);

int loadQuery4 (peatonesADT tad, FILE* query4);

//  Recibe un string y devuelve 1 si es un numero y 0 en caso contrario
int stringIsNumber (const char * num);

// Funcion auxiliar que devuelve el numero del mes
int monthToNum (char * month);
// Funcion auxiliar que pasa al siguiente token
char * update (const char * token);

int main(int argc, char * argv[]){

    // Validacion de parametros (EINVAL: argumento invalido)
    int yearRange[FROM_TO]={0, 0};
    switch (argc) {
        case 3:
            break;
        case 5:
            yearRange[TO] = atoi(argv[RANGE2]);
        case 4:
            yearRange[FROM] = atoi(argv[RANGE1]);
            if(yearRange[1] != 0 && yearRange[TO] < yearRange[FROM])
                errorExit(EINVAL, "arg[FROM] > arg[TO]", argv[FILENAME]);
            break;
        default:
            errorExit(EINVAL, "Cantidad invalida de argumentos", argv[FILENAME]);
            break;
    }


// **************************************************** DECLARACION DE ARCHIVOS **********************************************************************************

    FILE * dataSensors = fopen(argv[2], "r");
    FILE * dataReadings= fopen(argv[1], "r");
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

    if(status == EFILE){
        closeExit(files, EINVAL, "ERROR : durante la carga de datos, el archivo ingresado esta vacio", argv[FILENAME], fileCount, tad);
    }
    if(status == ENOMEM){
        errorExit(ENOMEM, "ERROR : No hay memoria suficiente en el heap", argv[FILENAME]);
    }
    // UNA VEZ QUE YA CARGAMOS TODOS LOS DATOS, LIMPIAMOS EL VECTOR

    eliminaCeros(tad);

    // EJECUTAMOS QUERIES

// CARGA DE DATOS A LOS ARCHIVOS QUERY 1, 2, 3 y 4
    if (loadQuery1 (tad, query1) < 0)
        closeExit(files, EFILE, "Hubo un error en la carga del query1", argv[0], fileCount, tad);

    if (loadQuery2 (tad, query2) < 0)
        closeExit(files, EFILE, "Hubo un error en la carga del query2", argv[0], fileCount, tad);

    if (loadQuery3 (tad, query3) < 0)
        closeExit(files, EFILE, "Hubo un error en la carga del query3", argv[0], fileCount, tad);

    if (loadQuery4 (tad, query4) < 0)
        closeExit(files, EFILE, "Hubo un error en la carga del query4", argv[0], fileCount, tad);

    closeAllFiles(files, fileCount);
    freePeatones(tad);
}

int fillAdt(peatonesADT tad, FILE* dataSensors, FILE* dataReadings, int * yearRange){

    //  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    int id;
    char buff[BUFF_SIZE], * token, * name; // en buff se van a ir llegando las lineas del .csv. BUFF_SIZE es un tamaño arbitrario

    // Si la primer linea del archivo dataSensors esta vacia, retorna un mensaje de error y aborta el programa
    if (fgets(buff, BUFF_SIZE, dataSensors) == NULL) {
        return EFILE;
    }

    while (fgets(buff, BUFF_SIZE, dataSensors) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
        id = atoi(token);
        if(!sensorExists(tad, id)) { // si el id no es duplicado
            token = update(token);
            name = token;
            token = update(token);
            if (token[0] == 'A'){

                putSensor(tad, id, name); // creo los sensores
            }
        }
    }

// *************************************************************** DATA READINGS ************************************************************************************

//  VARIABLES QUE LLENAMOS CON DATA_READINGS
    char * Wday;
    int sensorId, counts, status;
    int dateFormatted[DATE_FIELDS];
    if (fgets(buff, BUFF_SIZE, dataReadings) == NULL) {
        return EFILE;
    }

    while (fgets(buff, BUFF_SIZE, dataReadings) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
        dateFormatted[YEAR] = atoi(token);
        token = update(token);
        dateFormatted[MONTH] = monthToNum(token);
        token = update(token);
        dateFormatted[DAY] = atoi(token);
        token = update(token);
        Wday = token;
        token = update(token);
        sensorId = atoi(token);
        token = update(token);
        dateFormatted[HOUR] = atoi(token);
        token = update(token);
        counts = atoi(token);
        token = update(token); //token vale NULL

        if (sensorExists(tad, sensorId)) {     // se ignoran los sensores repetidos y desactivados
            status = addReading(tad, sensorId, dateFormatted, Wday, counts, yearRange); // creo los sensores
            if(status == ENOMEM){
                return ENOMEM;
            }
        }
    }
    return OK;
}

int loadQuery1 (peatonesADT tad, FILE * query1){
    sortTotal(tad);
    long int count;
    char * name;
    for (int i=1; i <= getCantSensores(tad); i++) {
        count = getSensorCount(tad, i);
        name = getNameById(tad, i);
        if (name == NULL || count == EID) {
            return EID;
        }
        int res = fprintf(query1, "%s;%li\n", name, count);
        if (res < 0)
            return EFILE;
    }
    return OK;
}
//función que escribe sobre el archivo query2.csv con los resultados de la consulta
//devuelve 1 si no hubo problemas en la escritura del archivo o 0 si hubo un error
int loadQuery2 (peatonesADT tad, FILE * query2){
    toBeginYear(tad);
    int year;
    long int counts;
    while (hasNextYear(tad)){
        year = getYear(tad);
        counts = getCount(tad);
        int res = fprintf(query2, "%d;%li\n", year, counts);;
        if (res < 0){
            return EFILE;
        }
        nextYear(tad);
    }
    return OK;
}

int loadQuery3 (peatonesADT tad, FILE * query3){
    long int nightCount, dayCount;
    for (int day = MONDAY; day<CANT_DAYS; day++){
        nightCount = getDailyCount(tad, day, TRUE);
        dayCount = getDailyCount(tad, day, FALSE);
        if ( nightCount < 0 || dayCount < 0){
            return EDAY;
        }
        int res = fprintf(query3, "%d;%li;%li;%li\n", day, dayCount, nightCount, dayCount+nightCount);
        if (res < 0)
            return EFILE;
    }
    return OK;
}

int loadQuery4 (peatonesADT tad, FILE * query4){
    sortMax(tad);
    int count;
    char* name;
    int dateFormatted[DATE_FIELDS];
    for (int i=1; i <= getCantSensores(tad); i++){
        count = getMaxCount(tad, i);
        if(count == 0) return IGNORE;
        name = getNameById(tad, i);
        getDate(tad, i, dateFormatted);
        int res = fprintf(query4, "%s;%d;%d;%d/%d/%d\n", name, count, dateFormatted[3], dateFormatted[0], dateFormatted[1], dateFormatted[2]);
        if (res < 0)
            return EFILE;
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

char * update (const char * token){
    return strtok(NULL, DELIM_FIELD);
}

int monthToNum (char * month){
    char * months[CANT_MONTH] = {"January", "February", "March","April","May", "June", "July",
                                 "August", "September", "October", "November", "December"};
    for (int i=0; i<CANT_MONTH; i++){
        if (strcmp(month, months[i])==0)
            return i+1;
    }
    return -1;
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


