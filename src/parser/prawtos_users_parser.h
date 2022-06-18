#ifndef PRAWTOS_HELLO_PARSER_H
#define PRAWTOS_HELLO_PARSER_H

#define MAX_LEN 0xFF

// Agregar usuario:
//             +------+-------+-----+------+----------+------+----------+
//             | TYP  |  CMD  |ADMIN| ULEN |   UNAME  | PLEN |  PASSWD  |
//             +------+-------+-----+------+----------+------+----------+
//             |   1  |   1   |  1  |   1  | 1 to 255 |   1  | 1 to 255 |
//             +------+-------+-----+------+----------+------+----------+
//      Donde:

//           o  TYP    tipo de comando: X'01' USERS
//           o  CMD    comando solicitado: X'01' CREATE
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
//           o  CMD    comando solicitado: X'02' DELETE 
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
//           o  CMD    comando solicitado: X'03' EDIT
//           o  ULEN   longitud del campo UNAME
//           o  UNAME  nombre del usuario
//           o  PLEN   longitud del campo PAWSSWD
//           o  PLEN   contraseña

typedef struct user
{
    bool admin;
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    
} user;

enum pt_type{
    type_get = 0x00,
    type_users = 0x01,
    type_sniff = 0x02,
};

enum pt_cmd{
    cmd_create = 0x01,
    cmd_delete = 0x02,
    cmd_edit = 0x03,
};

enum pt_state{
    pt_type,
    pt_cmd,
    pt_admin,
    pt_ulen,
    pt_uname,
    pt_plen,
    pt_passwd,

    // done section
    pt_done,

    // error section
    pt_error_unsupported_type,
    pt_error_unsupported_cmd,
    pt_error,
};

typedef struct cmd_parser{ 
    enum pt_state state;
    enum pt_type type;
    enum pt_cmd cmd;

    user * user;
    uint8_t read;
    uint8_t remaining;
} cmd_parser;

#endif