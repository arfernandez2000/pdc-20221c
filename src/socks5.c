#include "../include/socks5.h"
#include "../include/buffer.h"
#include "../include/args.h"
#include "../include/stm_initialize.h"
#include "../include/register.h"
#include "../include/stadistics.h"

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
    client_handler.handle_block = NULL;

    server_handler.handle_read = server_read;
    server_handler.handle_write = server_write;
    server_handler.handle_close = server_close;
    server_handler.handle_block = NULL;

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

    uint8_t *inputBuffer = malloc(inputBufferSize*sizeof(*inputBuffer));
    if(inputBuffer == NULL){
        free(session);
        return NULL;
    }
        
    uint8_t *outputBuffer = malloc(outputBufferSize*sizeof(*outputBuffer));
    if(outputBuffer == NULL){
        free(session);
        free(inputBuffer);
        return NULL;
    }

    // session->s_machine = (state_machine) malloc(sizeof(state_machine));
    // if(session->s_machine == NULL){
    //     free(session);
    //     free(inputBuffer);
    //     free(outputBuffer);
    //     return NULL;
    // }
    // session->s_machine->states = malloc(sizeof(state_definition));
    // if(session->s_machine->states == NULL){
    //     free(session);
    //     free(inputBuffer);
    //     free(outputBuffer);
    //     free(session->s_machine);
    //     return NULL;
    // }
    // session->s_machine->current = malloc(sizeof(state_definition));
    // if(session->s_machine->current == NULL){
    //     free(session);
    //     free(inputBuffer);
    //     free(outputBuffer);
    //     free(session->s_machine->states);
    //     free(session->s_machine)
    //     return NULL;
    // }
    // session->client_information = malloc(sizeof(Client));
    // if(session->client_information == NULL){
    //     free(session);
    //     free(inputBuffer);
    //     free(outputBuffer);
    //     free(session->s_machine->states);
    //     free(session->s_machine->current);
    //     free(session->s_machine);
    //     return NULL;
    // }
    // session->client = malloc(sizeof(Connection));
    // if(session->client == NULL){
    //     free(session);
    //     free(inputBuffer);
    //     free(outputBuffer);
    //     free(session->s_machine->states);
    //     free(session->s_machine->current);
    //     free(session->s_machine);
    //     free(session->client_information);
    //     return NULL;
    // }


    buffer_init(&session->input, inputBufferSize, inputBuffer);
    buffer_init(&session->output, outputBufferSize, outputBuffer);
    stm_create(&(session->s_machine));
    // initialize_state_machine(&session->s_machine);
    
    

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
        close_session(event);
    }
    
}

void client_read(selector_key  *event)
{
    state_machine * stm = &((Session *) event->data)->s_machine;
    const enum session_state st = stm_handler_read(stm, event);

    if (ERROR == st || DONE == st)
    {
        close_session(event);
    }
}

void client_close(selector_key *event){
    Session * session = (Session * ) event->data;
    
    close(session->client.fd);
    free(session->input.data);
    free(session->output.data);
    free(session);
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
            close_session(event);
        }
    }
}

static void server_close(selector_key * event){
    Session * session = (Session *) event->data;
    close(event->fd);
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
            close_session(event);
        }
    }   
}