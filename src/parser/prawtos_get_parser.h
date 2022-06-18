#ifndef PRAWTOS_GET_PARSER_H
#define PRAWTOS_GET_PARSER_H
#include <stdint.h>
#include <stdbool.h>
#include "../buffer/buffer.h"

/**
 *2. Obtencion de metricas:

   2.1 Pedido GET:
                    +----+-----+
                    |TYP | CMD |
                    +----+-----+
                    | 1  |  1  |
                    +----+-----+

     Donde:

          o  TYP    tipo de comando: X'00' GET
          o  CMD
             o  GET bytes transferidos X'01'
             o  GET conexiones historicas X'02'
             o  GET conexiones concurrentes X'03'
             o  GET listar usuarios X'04'
 * 
 */


enum get_state {
    get_type,
    get_cmd,

    get_done,

    get_error,
    get_error_unsupported_type,
    get_error_unsupported_cmd,
};

enum get_cmd {
    get_transfered,
    get_historical,
    get_concurrent,
    get_users,
};

typedef struct get {
    enum get_cmd cmd;
} get;


typedef struct get_parser {

    get * get;
    enum get_state state;

    //cuantos bytes tenemos que leer
    uint8_t n;
    //cuantos bytes ya leimos
    uint8_t i;
    
} get_parser;

/** inicializa el parser */
void
get_parser_init (get_parser *p);

/** entreha un byte al parser. Retorna true si se llego al final */
enum get_state
get_parser_feed (get_parser *p, const uint8_t c);

/** consume los bytes del mensaje del cliente y se los entrega al parser 
 * hasta que se termine de parsear 
**/
enum get_state
get_consume(buffer *b, get_parser *p, bool *errored);

bool
get_is_done(const enum get_state st, bool *errored);

void 
get_close(get_parser *p);

/**
 *2.2 Respuesta sobre el pedido:
        +--------+----+--------+--------+
        | STATUS | CMD| NARGS  |  ARGS  |
        +--------+----+--------+--------+
        |    1   | 1  |1 to 255|variable|
        +--------+----+--------+--------+

     Donde:
         o  STATUS estado de la conexion
             o  X'00' operacion exitosa
             o  X'01' fallo del servidor  
             o  X'02' tipo de comando no soportado       
             o  X'03' comando no soportado
          o  CMD    comando solicitado: 
             o  GET bytes transferidos X'01'
             o  GET conexiones historicas X'02'
             o  GET conexiones concurrentes X'03'
             o  GET listar usuarios X'04
          o  NARGS    cantidad de argumentos del campo ARGS
          o  ARGS     argumentos del comando solicitado, 
                      cuyo primer byte corresponde a la longitud del argumento
 */

enum get_response_status {
    get_success,
    get_server_failure,
    get_cmd_not_supported,
    get_type_not_supported,
};

int 
get_marshal(buffer *b, const enum get_response_status status, const enum get_cmd cmd, uint8_t nargs, uint8_t* args);


#endif