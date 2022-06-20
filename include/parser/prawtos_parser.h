
#ifndef PRAWTOS_PARSER_H
#define PRAWTOS_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "../buffer.h"

#define MAX_LEN 0xFF

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

// Agregar usuario:
//             +------+-------+-----+------+----------+------+----------+
//             | TYP  |  CMD  |ADMIN| ULEN |   UNAME  | PLEN |  PASSWD  |
//             +------+-------+-----+------+----------+------+----------+
//             |   1  |   1   |  1  |   1  | 1 to 255 |   1  | 1 to 255 |
//             +------+-------+-----+------+----------+------+----------+
//      Donde:

//           o  TYP    tipo de comando: X'01' USERS
//           o  CMD    comando solicitado: X'00' CREATE
//           o  ADMIN  flag para indicar tipo de usuario
//              o  X'00' admin
//              o  X'01' user
//           o  ULEN   longitud del campo UNAME
//           o  UNAME  nombre del usuario
//           o  PLEN   longitud del campo PAWSSWD
//           o  PLEN   contraseña
          
// Baja de usuario:
//             +------+-------+-----+---------+
//             | TYP  |  CMD  |ULEN | UNAME   |
//             +------+-------+-----+---------+
//             |   1  |   1   |  1  |1 to 255 |
//             +------+-------+-----+---------+
//      Donde:

//           o  TYP    tipo de comando: X'01' USERS
//           o  CMD    comando solicitado: X'01' DELETE 
//           o  ULEN   longitud del campo UNAME
//           o  UNAME  nombre del usuario

// Modificacion de usuario y contraseña:
//             +-------+-----+------+----------+------+----------+
//             |  TYP  | CMD | ULEN |   UNAME  | PLEN |  PASSWD  |
//             +-------+-----+------+----------+------+----------+
//             |   1   |  1  |   1  | 1 to 255 |   1  | 1 to 255 |
//             +-------+-----+------+----------+------+----------+
//      Donde:

//           o  TYP    tipo de comando: X'01' USERS
//           o  CMD    comando solicitado: X'02' EDIT
//           o  ULEN   longitud del campo UNAME
//           o  UNAME  nombre del usuario
//           o  PLEN   longitud del campo PAWSSWD
//           o  PLEN   contraseña

/**
 * 4. Configuracion remota:
   4.1 Habilitar/deshabilitar el sniffer de contraseñas:
            +-----+-----+
            | TYP | CMD |
            +-----+-----+
            |  1  |  1  |
            +-----+-----+
      Donde:

          o  TYP    tipo de comando: X'02' SNIFF
          o  CMD  flag para habilitar o deshabilitar el sniffeo de contraseñas
             o  X'00' habilitar
             o  X'01' deshabilitar       

5. Cerrar cliente:
            +-----+
            | TYP |
            +-----+
            |  1  |
            +-----+
      Donde:
          o  TYP    tipo de comando: X'03' QUIT  
 * 
 */

enum user_cmd{
    user_create     = 0x00,
    user_delete     = 0x01,
    user_edit       = 0x02,
};

enum get_cmd {
    get_transfered  = 0x00,
    get_historical  = 0x01,
    get_concurrent  = 0x02,
    get_users       = 0x03,
};

enum sniff_cmd {
    sniff_on    = 0x00,
    sniff_off   = 0x01,
};

enum admin {
    user    = 0x00,
    admin   = 0x01,
};

enum prawtos_parser_state {
    //compartido
    prawtos_type,
    prawtos_cmd_get,
    prawtos_cmd_user,
    prawtos_cmd_sniff,
    //user
    prawtos_admin,
    prawtos_ulen,
    prawtos_uname,
    prawtos_plen,
    prawtos_passwd,

    prawtos_parser_done,

    prawtos_error,
    prawtos_error_unsupported_type,
    prawtos_error_unsupported_cmd,
    prawtos_error_unsupported_admin,
};

typedef struct user_st {
    enum user_cmd cmd;
    enum admin admin;
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    
} user_st;

typedef struct get {
    enum get_cmd cmd;
} get;

typedef struct sniff_st {
    enum sniff_cmd cmd;
} sniff_st;

typedef struct prawtos_parser {

    get * get;
    user_st * user;
    sniff_st * sniff;
    uint8_t type;
    enum prawtos_parser_state state;

    //cuantos bytes tenemos que leer
    uint8_t n;
    //cuantos bytes ya leimos
    uint8_t i;
    
} prawtos_parser;

/** inicializa el parser */
void
prawtos_parser_init (prawtos_parser *p);

/** entra de a un byte al parser. Retorna true si se llego al final */
enum prawtos_parser_state
prawtos_parser_feed (prawtos_parser *p, const uint8_t c);

/** consume los bytes del mensaje del cliente y se los entrega al parser 
 * hasta que se termine de parsear 
**/
enum prawtos_parser_state
prawtos_consume(buffer *b, prawtos_parser *p, bool *errored);

bool
prawtos_is_done(const enum prawtos_parser_state st, bool *errored);


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

enum prawtos_response_status {
    success,
    server_failure,
    cmd_not_supported,
    type_not_supported,
    user_lens_error,
    user_credentia,
};

int 
get_marshal(buffer *b, const enum prawtos_response_status status, const enum get_cmd cmd, uint8_t nargs, uint8_t* args, int args_len);

/**
 * 3.4 Respuesta sobre usuario:
               +------+
               |STATUS|
               +------+
               |  1   |
               +------+
      Donde:

            o  STATUS estado sobre el pedido
               o  X'00' operacion exitosa
               o  X'01' fallo del servidor
               o  X'02' tipo de comando no soportado
               o  X'03' comando no soportado
               o  X'04' error en longitud para campos de usuario
               o  X'05' error en credenciales de usuario/no existe usuario
 */


int 
user_marshal(buffer *b, const enum prawtos_response_status status);

int 
quit_marshal(buffer *b, const enum prawtos_response_status status);

#endif