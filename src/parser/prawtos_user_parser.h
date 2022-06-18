#ifndef PRAWTOS_USER_PARSER_H
#define PRAWTOS_USER_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "../buffer/buffer.h"

#define MAX_LEN 0xFF

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

enum user_cmd{
    user_create = 0x00 ,
    user_delete = 0x01 ,
    user_edit = 0x02,
};

enum admin {
    admin = 0x00,
    user = 0x01,
};

enum user_state {
    user_type,
    user_cmd,
    user_admin,
    user_ulen,
    user_uname,
    user_plen,
    user_passwd,

    // done section
    user_done,

    // error section
    user_error_unsupported_type,
    user_error_unsupported_cmd,
    user_error_unsupported_admin,
    user_error,
};

typedef struct user_st {
    enum user_cmd cmd;
    enum admin admin;
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    
} user_st;

typedef struct user_parser { 

    user_st * user;
    enum user_state state;

    //cuantos bytes tenemos que leer
    uint8_t n;
    //cuantos bytes ya leimos
    uint8_t i;

} user_parser;

/** inicializa el parser */
void
user_parser_init (user_parser *p);

/** entreha un byte al parser. Retorna true si se llego al final */
enum user_state
user_parser_feed (user_parser *p, const uint8_t c);

/** consume los bytes del mensaje del cliente y se los entrega al parser 
 * hasta que se termine de parsear 
**/
enum user_state
user_consume(buffer *b, user_parser *p, bool *errored);

bool
user_is_done(const enum user_state st, bool *errored);

void 
user_close(user_parser *p);

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

enum user_response_status {
    user_success,
    user_server_failure,
    user_cmd_not_supported,
    user_type_not_supported,
    user_lens_error,
    user_credentia,
};

int 
user_marshal(buffer *b, const enum user_response_status status);

#endif