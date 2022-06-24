#include "../include/socks5.h"
#include "../include/buffer.h"
#include "../include/args.h"
#include "../include/stm_initialize.h"
#include "../include/register.h"
#include "../include/stadistics.h"

static void server_write(selector_key * event);
static void server_read(selector_key * event);
static void server_close(selector_key * event);
static void server_done(selector_key * event);
static void close_session(selector_key * event);
static void server_block(selector_key *event);

static Session* initialize_session();


static int inputBufferSize;
static int outputBufferSize;
static fd_handler client_handler;
static fd_handler server_handler;

void initialize_socks5(socks5args args,fd_selector selector) {
    
    inputBufferSize = INPUT_BUFFER_SIZE;
    outputBufferSize = OUTPUT_BUFFER_SIZE;

    client_handler.handle_read = client_read;
    client_handler.handle_write = client_write;
    client_handler.handle_close = client_close;
    client_handler.handle_block = server_block;

    server_handler.handle_read = server_read;
    server_handler.handle_write = server_write;
    server_handler.handle_close = server_close;
    server_handler.handle_block = server_block;

    // Aca habria que inicializar la maquina de estados.
    stm_map();
}


void new_connection_ipv4(selector_key *event) {

    struct sockaddr_storage cli_address;
    socklen_t clilen = sizeof(cli_address);

    int fd;

    do {
        fd = accept(event->fd, (struct sockaddr *)&cli_address, &clilen);
    } while (fd < 0 && (errno == EINTR));

    Session * session = initialize_session();  
    if (session == NULL) {
        perror("somos unos perrors");
        close(fd);
        return;
    }

    session->lastModified = time(NULL);

    session->client.fd = fd;
    session->server.fd = -1;

    memcpy(&session->client.address, (struct sockaddr *)&cli_address, clilen);
    
    session->client.address_len = clilen;
    session->register_info.client_addr = cli_address;

    selector_register(event->s, session->client.fd, &client_handler, OP_READ, session);

    stadistics_increase_concurrent();
}

void new_connection_ipv6(selector_key *event) {

    struct sockaddr_storage cli_address;
    socklen_t clilen = sizeof(cli_address);

    int fd;

    do {
        fd = accept(event->fd, (struct sockaddr *)&cli_address, &clilen);
    } while (fd < 0 && (errno == EINTR));


    Session * session = initialize_session();  
    if (session == NULL) {
        perror("somos unos perrors");
        close(fd);
        return;
    }

    session->lastModified = time(NULL);

    session->client.fd = fd;
    session->server.fd = -1;

    memcpy(&session->client.address, (struct sockaddr *)&cli_address, clilen);
    session->client.address_len = clilen;
    session->register_info.client_addr = cli_address;

    selector_register(event->s, session->client.fd, &client_handler, OP_READ, session);
    
    stadistics_increase_concurrent();
}

static Session* initialize_session() {
    Session* session = malloc(sizeof(*session));
    if(session == NULL){
        return NULL;
    }
    memset(session, 0x00, sizeof(*session));

    session->references = 1;
    buffer_init(&session->input, sizeof(session->raw_buff_a)/sizeof((session->raw_buff_a)[0]), session->raw_buff_a);
    buffer_init(&session->output, sizeof(session->raw_buff_b)/sizeof((session->raw_buff_b)[0]), session->raw_buff_b);
    stm_create(&(session->s_machine));
    
    

    return session;
}

// static void initialize_state_machine(state_machine* s_machine) {
    // state_definition s_definition[FINISH + 1];
// }

void client_write(selector_key * event){

    
    state_machine * stm = &((Session *) event->data)->s_machine;
    const enum session_state st = stm_handler_write(stm, event);

    if (ERROR == st || DONE == st)
    {
        server_done(event);
    }
    
}

void client_read(selector_key  *event)
{
    state_machine * stm = &((Session *) event->data)->s_machine;
    const enum session_state st = stm_handler_read(stm, event);

    if (ERROR == st || DONE == st)
    {
        server_done(event);
    }
}

void client_close(selector_key *event){
    Session * session = (Session * ) event->data;
    if(session->references == 1){
    
        if (session->origin_resolution != NULL) {
            freeaddrinfo(session->origin_resolution);
            session->origin_resolution = 0;
        }
        free(session);
    }
    else{
        session->references -=1;
    }
}
    
static void close_session(selector_key * event){
    Session * session = (Session *) event->data;
    selector_unregister_fd(event->s, session->client.fd);
    if(get_concurrent_connections() > 0)
        stadistics_decrease_concurrent();
    close(session->client.fd);
    //free(session->client);
    //free(session);

}

static void server_read(selector_key * event){
    Session * session = (Session *) event->data;
    session->lastModified = time(NULL);

    buffer * buffer_read = &session->output;
    if(!buffer_can_write(buffer_read)){
        return;
    }

    ssize_t readBytes;
    size_t nbytes;
    uint8_t * writePtr = buffer_write_ptr(buffer_read, &nbytes);

    if(readBytes = recv(event->fd, writePtr, nbytes, MSG_NOSIGNAL), readBytes >= 0) {
        buffer_write_adv(buffer_read, readBytes);
    }
    else {
        if(errno != EINTR) {
            server_done(event);
        }
    }
}

static void server_close(selector_key * event){
    //Session * session = (Session *) event->data;
    close(event->fd);
}

static void server_done(struct selector_key *event) {
    const int fds[] = {
        ((Session *) event->data)->client.fd,
        ((Session *) event->data)->server.fd,
    };

    for (unsigned i = 0; i < (sizeof(fds)/sizeof(fds[0])); i++) {
        if (fds[i] != -1) {
            if (SELECTOR_SUCCESS != selector_unregister_fd(event->s, fds[i])) {
                abort();
            }
            close(fds[i]);
        }
    }

    stadistics_decrease_concurrent();
}

static void server_block(selector_key *event) {
    struct state_machine *stm = &((Session *) event->data)->s_machine;
    const enum session_state st = stm_handler_block(stm, event);

    if (ERROR == st || DONE == st) {
        server_done(event);
    }
}

static void server_write(selector_key * event){
    Session * session = (Session *) event->data;
    session->lastModified = time(NULL);

    buffer * buffer_write = &session->output;

    if(!buffer_can_read(buffer_write)){
        return;
    }
    
    size_t write_bytes;
    size_t bytes;
    uint8_t * read = buffer_read_ptr(buffer_write, &bytes);
    
    if(write_bytes = send(event->fd, read, bytes, MSG_NOSIGNAL), write_bytes > 0){
        buffer_read_adv(buffer_write, write_bytes);
        stadistics_increase_bytes_sent(bytes);
    }
    else{
        if(errno!=EINTR){
            server_done(event);
        }
    }   
}