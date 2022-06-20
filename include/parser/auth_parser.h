#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H

#include <stdint.h>
#include "../buffer.h"

#define MAX_LEN 0xFF
#define AUTH_SUCCESS 0x00
#define AUTH_BAD_CREDENTIALS 0X03

/**
 * This begins with the client producing a
   Username/Password request:

           +----+------+----------+------+----------+
           |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
           +----+------+----------+------+----------+
           | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
           +----+------+----------+------+----------+
   The VER field contains the current version of the subnegotiation,
   which is X'01'. The ULEN field contains the length of the UNAME field
   that follows. The UNAME field contains the username as known to the
   source operating system. The PLEN field contains the length of the
   PASSWD field that follows. The PASSWD field contains the password
   association with the given UNAME.
 * 
 */

typedef struct auth
{
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    
} auth;

enum auth_state {
    auth_version,
    auth_ulen,
    auth_uname,
    auth_plen,
    auth_passwd,

    //a partit de aca estan done
    auth_done,

    //a partir de aca son considerados con error
    auth_error,
    auth_error_unsupported_version,
};

typedef struct auth_parser
{
    auth * auth;
    enum auth_state state;

    //cuantos bytes tenemos que leer
    uint8_t n;
    //cuantos bytes ya leimos
    uint8_t i;

} auth_parser;

/** inicializa el parser */
void
auth_parser_init (auth_parser *p);

/** entreha un byte al parser. Retorna true si se llego al final */
enum auth_state 
auth_parser_feed(auth_parser *p, const uint8_t c);

/** consume los bytes del mensaje del cliente y se los entrega al parser 
 * hasta que se termine de parsear 
**/
enum auth_state 
auth_consume(buffer *b, auth_parser *p, bool *errored);

bool 
auth_is_done(const enum auth_state state, bool *errored);

/** The server verifies the supplied UNAME and PASSWD, and sends the
   following response:

            +----+--------+
            |VER | STATUS |
            +----+--------+
            | 1  |   1    |
            +----+--------+

   A STATUS field of X'00' indicates success. If the server returns a
   `failure' (STATUS value other than X'00') status, it MUST close the
   connection.
*/

/** ensambla la respuesta del hello dentro del buffer con el metodo 
 * seleccionado.
**/
int 
auth_marshal(buffer *b, const uint8_t status);




#endif
