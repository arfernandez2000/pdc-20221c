#include "socks5.h"

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
}


void new_connection_ipv4(selector_key *event) {

    struct sockaddr_in cli_address;
    socklen_t clilen = sizeof(cli_address);

    int fd;

    do {
        fd = accept(event->fd, (struct sockaddr *)&cli_address, &clilen);
    } while (fd < 0 && (errno == EINTR));

    fprintf(stdout,"Mi file descriptor es: %d\n", fd);

    Session * session = initialize_session();  
    if (session == NULL) {
        perror("somos unos perrors");
        close(fd);
        return;
    }

    session->lastModified = time(NULL);

    session->client.fd = fd;

    memcpy(&session->client.address, (struct sockaddr *)&cli_address, clilen);

    selector_register(event->s, session->client.fd, &client_handler, OP_READ, session);

}

static Session* initialize_session() {
    Session* session = calloc(1, sizeof(*session));
    if(session == NULL){
        return NULL;
    }

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

    buffer_init(&session->input, inputBufferSize, inputBuffer);
    buffer_init(&session->output, outputBufferSize, outputBuffer);

    // initialize_state_machine(&session->s_machine);
    
    

    return session;
}

// static void initialize_state_machine(state_machine* s_machine) {
    // state_definition s_definition[FINISH + 1];
// }

static void client_write(selector_key * event){

    fprintf(stdout,"Estoy en client_write!\n");
    
    Session * session = (Session *) event->data;
    session->lastModified = time(NULL);

    buffer * buffer_write = &session->output;

    if(!buffer_can_read(buffer_write)){
        return;
    }
    
    ssize_t write_bytes;
    size_t bytes;
    uint8_t * read = buffer_read_ptr(buffer_write, &bytes);
    
    if(write_bytes = send(event->fd, read, bytes, MSG_NOSIGNAL), write_bytes > 0){
        buffer_read_adv(buffer_write, write_bytes);
    }
    else{
        if(errno!=EINTR){
            close_session(event);
        }
    }
    
}

static void client_read(selector_key  *event)
{
    fprintf(stdout,"Estoy en client_read!\n");
    Session * session = (Session *) event->data;
    session->lastModified = time(NULL);
    buffer * buffer_read = &session->output;

    if(!buffer_can_read(buffer_read)){
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

static void client_close(selector_key *event){
    fprintf(stdout, "Estoy en el client_close");
    Session * session = (Session * ) event->data;
    
    close(session->client.fd);
    free(session->input.data);
    free(session->output.data);
    free(session);
}
    
static void close_session(selector_key * event){
    fprintf(stdout, "Estoy en el close_session");
    Session * session = (Session *) event->data;
    selector_unregister_fd(event->s, session->client.fd);

}

static void server_read(selector_key * event){
    fprintf(stdout, "Estoy en el server_read");
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
    fprintf(stdout, "Estoy en el server_close");
    Session * session = (Session *) event->data;
    close(event->fd);
}

static void server_write(selector_key * event){
    fprintf(stdout, "Estoy en el server_write");
    Session * session = (Session *) event->data;
    session->lastModified = time(NULL);

    buffer * buffer_write = &session->output;

    if(!buffer_can_read(buffer_write)){
        return;
    }
    
    ssize_t write_bytes;
    size_t bytes;
    uint8_t * read = buffer_read_ptr(buffer_write, &bytes);
    
    if(write_bytes = send(event->fd, read, bytes, MSG_NOSIGNAL), write_bytes > 0){
        buffer_read_adv(buffer_write, write_bytes);
    }
    else{
        if(errno!=EINTR){
            close_session(event);
        }
    }
    
}