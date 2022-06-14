#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "../buffer/buffer.h"

/* The SOCKS request is formed as follows:

        +----+-----+-------+------+----------+----------+
        |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

     Where:

          o  VER    protocol version: X'05'
          o  CMD
             o  CONNECT X'01'
             o  BIND X'02'
             o  UDP ASSOCIATE X'03'
          o  RSV    RESERVED
          o  ATYP   address type of following address
             o  IP V4 address: X'01'
             o  DOMAINNAME: X'03'
             o  IP V6 address: X'04'
          o  DST.ADDR       desired destination address
          o  DST.PORT desired destination port in network octet
             order
*/

enum socks_req_cmd {
    socks_req_cmd_connect =     0x01,
    socks_req_cmd_bind =        0x02,
    socks_req_cmd_associate =    0x03,
};

enum socks_addr_type {
    socks_addr_type_ipv4 =      0x01,
    socks_addr_type_domain =    0x03,
    socks_addr_type_ipv6 =      0x04,
};

union socks_addr {
    char fdqn[0xff];
    struct sockaddr_in  ipv4;
    struct sockaddr_in6 ipv6;
};

struct request {
    enum socks_req_cmd      cmd;
    enum socks_addr_type    dest_addr_type;
    union socks_addr        dest_addr;
    //port in network byte order
    in_port_t               dest_port;
};

enum request_state {
    request_version,
    request_cmd,
    request_rvs,
    request_atype,
    request_dstaddr_fqdn,
    request_dstaddr,
    request_dstport,

    //a partir de aca estan done
    request_done,

    //a partir de aca son considerados con error
    request_error,
    request_error_unsupported_version,
    request_error_unsupported_atype,
};

struct request_parser {
    struct request *request;

    enum request_state state;
    //cuantos bytes tenemos que leer
    uint8_t n;
    //cuantos bytes ya leimos
    uint8_t i;
};

/*Reply field:
    o  X'00' succeeded
    o  X'01' general SOCKS server failure
    o  X'02' connection not allowed by ruleset
    o  X'03' Network unreachable
    o  X'04' Host unreachable
    o  X'05' Connection refused
    o  X'06' TTL expired
    o  X'07' Command not supported
    o  X'08' Address type not supported
    o  X'09' to X'FF' unassigned
*/

enum socks_response_status {
    status_succeeded = 0X00,
    status_general_SOCKS_server_failure = 0X01,
    status_connection_not_allowed_by_ruleset = 0x02,
    status_network_unreachable = 0x03,
    status_host_unreachable = 0x04,
    status_connection_refused = 0x05,
    status_TTL_expired = 0x06,
    status_command_not_supported = 0x07,
    status_address_type_not_supported = 0x08,
};

/** inicializa el parser */
void
request_parser_init (struct request_parser *p);

/** entreha un byte al parser. Retorna true si se llego al final */
enum request_state
request_parser_feed (struct request_parser *p, const uint8_t c);

/** consume los bytes del mensaje del cliente y se los entrega al parser 
 * hasta que se termine de parsear 
**/
enum request_state
request_consume(buffer *b, struct request_parser *p, bool *errored);

bool
request_is_done(const enum request_state st, bool *errored);

void 
request_close(struct request_parser *p);

extern int
request_marshall(buffer *b, const enum socks_response_status status);

enum socks_response_status
errno_to_socks(int e);

#include <netdb.h>
#include <arpa/inet.h>

enum socks_response_status
cmd_resolve(struct request* request, struct sockaddr ** originaddr, 
            socklen_t *originlen, int *domain);

#endif