#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../buffer/buffer.h"

//    The client connects to the server, and sends a version
//    identifier/method selection message:

//                    +----+----------+----------+
//                    |VER | NMETHODS | METHODS  |
//                    +----+----------+----------+
//                    | 1  |    1     | 1 to 255 |
//                    +----+----------+----------+

//    The VER field is set to X'05' for this version of the protocol.  The
//    NMETHODS field contains the number of method identifier octets that
//    appear in the METHODS field.

//    The server selects from one of the methods given in METHODS, and
//    sends a METHOD selection message:

//                          +----+--------+
//                          |VER | METHOD |
//                          +----+--------+
//                          | 1  |   1    |
//                          +----+--------+

//    If the selected METHOD is X'FF', none of the methods listed by the
//    client are acceptable, and the client MUST close the connection.

//    The values currently defined for METHOD are:

//           o  X'00' NO AUTHENTICATION REQUIRED
//           o  X'01' GSSAPI
//           o  X'02' USERNAME/PASSWORD
//           o  X'03' to X'7F' IANA ASSIGNED
//           o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
//           o  X'FF' NO ACCEPTABLE METHODS

//    The client and server then enter a method-specific sub-negotiation.


static const uint8_t METHOD_NO_AUTHENTICATION_REQUIRED = 0x00;
static const uint8_t METHOD_NO_ACCEPTABLE_METHODS = 0xFF;
static const uint8_t METHOD_USERNAME_PASSWORD = 0x02;

enum hello_state
{
    hello_version,
    hello_nmethods,
    hello_methods,
    hello_done,
    hello_error_unsupported_version,
};

struct hello_parser
{
    /** invocado cada vez que se presenta un nuevo método **/
    void (*on_authentication_method)(void *data, const uint8_t method);
    // void (*on_authentication_method)(struct hello_parser *parser, const uint8_t method);

    /** permite al usuario del parser almacenar sus datos **/
    void *data;

    /********* zona privada *********/
    enum hello_state state;
    /* cantidad de metodos que faltan por leer */
    uint8_t remaining;
};

/** inicializa el parser **/
void hello_parser_init(struct hello_parser *p);

/** entrega un byte al parser. Retorna true si se llego al final **/
enum hello_state hello_parser_feed(struct hello_parser *p, uint8_t b);

/** consume los bytes del mensaje del cliente y se los entrega al parser 
 * hasta que se termine de parsear 
**/
enum hello_state hello_consume(buffer *b, struct hello_parser *p, bool *error);

/** ensambla la respuesta del hello dentro del buffer con el metodo 
 * seleccionado.
**/
int hello_marshall(buffer *b, const uint8_t method);

bool hello_is_done(const enum hello_state state, bool *error);

//libera recursos internos del parser
void hello_parser_close(struct hello_parser *p);

/** en caso de que se haya llegado a un estado de error, permite 
 * obtener representacion textual que describe el problema 
 * */
extern const char * hello_error(const struct hello_parser *p);
#endif