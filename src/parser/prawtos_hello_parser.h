#ifndef PRAWTOS_HELLO_PARSER_H
#define PRAWTOS_HELLO_PARSER_H

/**
 * 1. Negociacion inicial:

   1.1 Pedido:
      +-----+------+----------+------+----------+
      | VER | ULEN |   UNAME  | PLEN |  PASSWD  |
      +-----+------+----------+------+----------+
      |  1  |   1  | 1 to 255 |   1  | 1 to 255 |
      +-----+------+----------+------+----------+
      
     Donde:

          o  VER    version del protocolo: X'01' VERSION 1
          o  ULEN   longitud del campo UNAME
          o  UNAME  nombre del usuario
          o  PLEN   longitud del campo PAWSSWD
          o  PLEN   contraseña
   
   1.2 Respuesta:
            +-----+------+
            | VER |STATUS|
            +-----+------+
            | 1   |  1   |
            +-----+------+
      Donde:

          o  VER    version del protocolo: X'01' VERSION 1
          o  STATUS estado de la conexion
             o  X'00' conexion exitosa
             o  X'01' fallo del servidor  
             o  X'02' version no soprtada        
             o  X'03' identificacion de usuario incorrecta
 * 
 */

#endif

