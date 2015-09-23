//
//  main.c
//  utob_client
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Communicator.h"
#include "Common.h"
#include "Consensus.h"
#include "UniformTotalOrderBroadcast.h"
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


FILE * fp;


static void catch_function(int signo) {
    printf("Signal %d\nReleasing resources\n",signo);
    fclose(fp);
    exit(0);
}


void onUTOB_Deliver(UTOB utob, int value, void * ctx){
    printf("Deliver: %d\n",value);
    fprintf(fp, "%d\n",value);

}


int main(int argc, const char * argv[]) {
    
    if (signal(SIGINT, catch_function) == SIG_ERR) {
        fputs("An error occurred while setting a signal handler.\n", stderr);
        return EXIT_FAILURE;
    }
        
    int serverPort = SERVER_BASE_PORT;
    int processes = 3;
    
    if (argc == 3) {
        serverPort = atoi(argv[1]);
        processes = atoi(argv[2]);
    }
    
    char tmp[50];
    sprintf(tmp, "order_%d.txt",serverPort);
    fp = fopen(tmp, "w+");
    srand(serverPort);
    
    UTOB utob = UTOB_Init(serverPort, processes);
    UTOB_Start(utob, onUTOB_Deliver, NULL);
    UTOB_Send(utob, serverPort);
    
    
    char temp[100];
    while (1) {
        char * num = fgets(temp, sizeof(temp), stdin);
        if (num == NULL) {
            return -1;
        }
        ssize_t len = strlen(num);
        if(len > 0 && num[len-1] == '\n'){
            num[len-1] = '\0';
        }
        UTOB_Send(utob, atoi(num));
    }
    
    return 0;
}
