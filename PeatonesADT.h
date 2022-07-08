/*
 * Creado por Uriel Angel Arias, Catalina Muller, Mateo Perez de Gracia y Sol Rodriguez
 *
 * Julio, 2022
 *
 * El proposito de este programa es organizar los datos de sensores de peatones. Este recibe informacion acerca de los
 * sensores(el nombre, ID, y estado -activo o removido-), con la cual arma un vector para luego almacenar los datos tomados
 * por cada uno de ellos. Recibe los datos tomdos por sensores de peatones y va almacenando la informacion segun distintos
 * criterios: segun el año en el que se tomo la medicion, segun el sensor que la tomo, o segun el dia de la semana en el
 * que se tomo, en este ultimo caso dividiendo a su vez si la medicion fue tomada durante el dia o la noche. Tambien
 * guarda por cada sensor cual fue la medicion maxima que tomo y en que fecha. Luego el programa porda devolver informacion
 * acerca de los sensores o mediciones (como el total de peatones que se midieron segun alguno de los criterios mencionados)
 */

#ifndef TPE_PEATONESADT_H
#define TPE_PEATONESADT_H

typedef struct peatonesCDT* peatonesADT;

typedef enum dateType { DAY=0, MONTH, YEAR, HOUR, DATE_FIELDS } dateType;

//definición de constantes simbolicas para errores del programa, no definidas en otras librerías utilizadas
#define E_NOT_FOUND (-1)    //Errores por elementos no encontrados/utilizados
#define E_FILE (-2)         //Errores de lectura/escritura de archivos
#define E_ID (-3)           //Error por id's de sensores inexistentes o repetidos
#define E_DAY (-4)          //Error por dias no validos dado daysType
#define E_NO_NEXT (-5)      //Error por intentar modificar lo apuntado por el iterador hacia un elemento fuera de la lista
#define OK 0

#define CANT_MONTH 12
#define MONDAY 0
#define CANT_DAYS 7


/*
 * Crea y devuelve un puntero a TAD vacío.
 * No recibe parámetros.
 */
peatonesADT newPeatones(void);

/*
 * Libera la memoria reservada para el TAD.
 * peat:    puntero al TAD.
 */
void freePeatones(peatonesADT peat);

/*
 * Agrega un sensor al vector del TAD, utilizando el ID para ubicarlo en el mismo y almacenando su nombre.
 * Devuelve OK si lo creó y E_ID si ya existe (no se modifica el vector).
 * id:      identificador del sensor.
 * name:    string null terminated con el nombre del sensor.
 */
int putSensor(peatonesADT pea, int id, char * name);

/*
 * Carga los datos de una medicion, siempre y cuando la misma este dentro del rango de años requerido por el usuario.
 * De ser asi, se incrementan los counts de cada categoria y el maxCount del respectivo sensor de ser una medicion maxima hasta el momento.
 * date[DATE_FIELDS]: Vector con los datos referidos a la fecha de la medicion, con el formato: date[DATE_FIELDS]= {DAY, MONTH, YEAR, HOUR}
 * day:               String Null Terminated con el nombre del día de la semana obtenido de la medicion.
 * counts:            Cantidad de peatones registrados en la medicion.
 * FromTo:            Vector con el rango de años a consultar, segun los parametros pasados por linea de comandos por el usuario.
 *                    En la posicion 0, se encuentra el anio de inicio (From) y en la posicion 1 el anio final (To).
 *                    Si no se especifico alguno de los datos del vector, se deja el valor 0 en la posición correspondiente.
 *                    Si el rango de anios no era valido, FromTo[0]=-1 y no se considera ningun anio como valido.
 */
int addReading(peatonesADT pea, int id, const int date[DATE_FIELDS], const char * day, int counts, const int FromTo[2]);

/*
 * Devuelve 1 si el sensor está en la lista de sensores activos y 0 en caso contrario.
 */
int sensorExists(peatonesADT pea, int id);

/*
 * Devuelve la cantidad de peatones que registró un sensor en función de su id.
 */
long int getSensorCount(peatonesADT pea, int id);

/*
 * Devuelve el nombre del sensor en función de su id.
 */
char * getNameById(peatonesADT pea, int id);

/*
 * Setea el iterador en el primer año de la lista.
 */
void toBeginYear(peatonesADT pea);

/*
 * modifica el iterador para que apunte al siguiente año de la lista.
 */
int nextYear(peatonesADT pea);

/*
 * Determina si hay un siguiente año en la lista respecto de la posición apuntada por el iterador.
 * Devuelve 1 si existe un siguiente y 0 en caso contrario.
 */
int hasNextYear(peatonesADT pea);

/*
 * Devuelve el año al que se encuentra apuntando el iterador.
 */
int getYear(peatonesADT pea);

/*
 * Devuelve el total de peatones registrado en el año al que apunta el iterador.
 */
long int getCount(peatonesADT pea);

/*
 * Devuelve la cantidad de sensores activos.
 */
unsigned long getSensorsAmount(peatonesADT pea);

/*
 *  Devuelve la cantidad de peatones en horario diurno/nocturno segun el dia y horario especificados.
 *  day:    valor que representa el día de la semana que se desea consultar.
 *  option: valor que indica el horario a revisar, donde 0 es para horario diurno (daylight) y 1 es para el nocturno (night).
 */
long int getDailyCount(peatonesADT pea, int day, char option);

/*
 * Completa el vector date con los datos de la medición maxima del sensor de id especificado.
 * Utiliza el formato: date[DATE_FIELDS]= {DAY, MONTH, YEAR, HOUR}
 * Devuelve OK si tuvo éxito o EID si no se encontró el sensor entre los activos.
 */
int getDate(peatonesADT pea, int id, int date[DATE_FIELDS]);

/*
 * Devuelve la cantidad de peatones de la medición máxima de un sensor especificado por id.
 */
int getMaxCount(peatonesADT pea, int id);

/*
 * Ordena descendentemente el vector de sensores en función de la medición maxima de cada uno.
 */
void sortMax(peatonesADT pea);

/*
 * Ordena descentamente el vector de sensores según la cantidad total de mediciones de cada uno.
 */
void sortTotal(peatonesADT pea);

/*
 * Reduce el vector de sensores de forma tal que las posiciones reservadas para id's que no están activos sean eliminadas
 * El vector resultante contiene solo sensores activos para su posterior reordenamiento.
 */
void deleteGaps(peatonesADT pea);

#endif //TPE_PEATONESADT_H
