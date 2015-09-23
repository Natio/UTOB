//
//  Communicator.c
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include "Communicator.h"
#include "Common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define TYPE_SERVER 1
#define TYPE_CLIENT 2
#define TYPE_UNDEFINED 0


typedef struct __Communicator {
    int socket;
    int server_port;
    
    struct handler{
        MessageHandler messageHandler;
        void * handlerContext;
    }handler[5];
    int handler_count;
   
    pthread_t reader_thread;
    pthread_mutex_t lock;
    int continue_receiving;
    int type;
    
}__Communicator;

void* communicationThread(void*);


Communicator CommunicatorInit(int port){
    Communicator self = malloc(sizeof(__Communicator));
    self->socket = -1;
    self->server_port = port;
    self->continue_receiving = 0;
    self->type = TYPE_UNDEFINED;
    pthread_mutex_init(&self->lock, NULL);
    return self;
}

long CommunicatorWrite(Communicator self, Message * buffer){
    pthread_mutex_lock(&self->lock);
    
    
    
    ssize_t written = 0;
    
    while (written < sizeof(Message)) {
        
        void * begin = (buffer) + written;
        
        ssize_t written_now = write(self->socket, begin, sizeof(Message) - written);
        if (written_now <=0) {
            pthread_mutex_unlock(&self->lock);
            printf("errore, scrittura non riuscita\n");
            return -1;
        }
        written+= written_now;
    }
    
    
    pthread_mutex_unlock(&self->lock);
    return written;
}

void CommunicatorAddHandler(Communicator self, MessageHandler hander, void * ctx){
    if (self->continue_receiving != 0) { //if not started
        printf("Cannot add an handler when communicator is started\n");
        exit(-1);
    }
    
    if (self->handler_count == 5) {
        printf("Maximum number of handlers reached\n");
        exit(-1);
    }
    self->handler[self->handler_count].messageHandler = hander;
    self->handler[self->handler_count].handlerContext = ctx;
    self->handler_count++;
}

void CommunicatorStartServer(Communicator c){
    pthread_mutex_lock(&c->lock);
    c->type = TYPE_SERVER;
    int sockfd, newsockfd;
    socklen_t clilen;
    
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("ERROR opening socket\n");
        exit(1);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(c->server_port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        printf("ERROR on binding to port: %d\n",c->server_port);
        exit(1);
    }
    listen(sockfd,1);
    printf("Listening on port %d\n", c->server_port);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    if (newsockfd < 0){
        printf("ERROR on accept\n");
        exit(1);
    }
    printf("Client connected!\n");
   
    close(sockfd);
    c->socket = newsockfd;
    c->continue_receiving = 1;
    pthread_mutex_unlock(&c->lock);
    pthread_create(&c->reader_thread, NULL, communicationThread, (void*)c);
   
}

void CommunicatorStartClient(Communicator c){
    c->type = TYPE_CLIENT;
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("ERROR opening socket\n");
        exit(1);
    }
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(c->server_port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        printf("ERROR connecting to port %d host: %s\n",c->server_port,server->h_name);
        printf("%s\n",strerror(errno));
        exit(1);
    }
    c->socket = sockfd;
    c->continue_receiving = 1;
    pthread_create(&c->reader_thread, NULL, communicationThread, (void*)c);
}




void* communicationThread(void* ptr){
    Communicator self = ptr;
    
    Message messageBuff;
    
    while (self->continue_receiving) {
        
        ssize_t read_size = 0;
        while (read_size < sizeof(Message)) {
            ssize_t size = read(self->socket, (&messageBuff +read_size), sizeof(Message) - read_size);
            if (size <= 0) {
                close(self->socket);
                if (self->type == TYPE_SERVER) {
                    for (int i = 0; i < self->handler_count; i++) {
                        self->handler[i].messageHandler(NULL, NULL);
                    }
                    pthread_exit(0);
                }
                else{
                    printf("Server disconnected. Killing app!!!\n");
                    exit(-1);
                }
                
            }
            read_size += size;
        }
        
        if (self->server_port == SERVER_BASE_PORT && self->type == SERVER_BASE_PORT) {
            printf("RCV: Type: %d \n", messageBuff.type);
        }
        
        for (int i = 0; i < self->handler_count; i++) {
            Message m = messageBuff;
            self->handler[i].messageHandler(&m, self->handler[i].handlerContext);
        }
        
    }
    
    
    
    
    return NULL;
}
