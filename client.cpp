/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: kumars
 *
 * Created on June 1, 2016, 1:28 PM
 */

#include <cstdlib>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ProtDefs.h"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}


/*
 * 
 */
int main(int argc, char** argv) {
    
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    
    request_header  reqh;
    response_header resh;
    
    char *keyV;
    char *buf_ptr;
     
     
     memset(&resh, 0, sizeof(resh));
     memset(&reqh, 0, sizeof(reqh));
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    server = gethostbyname(argv[1]);
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    //printf("Please enter the message: ");
    
    buf_ptr = buffer;
    
    bzero(buffer,256);
    
    //fgets(buffer,255,stdin);
    //keyV = "Hello";
    reqh.request.magic  = REQUEST;
    reqh.request.opcode = CMD_GET;
    //reqh.request.keylen = ntohs(strlen(keyV));
    //reqh.request.extlen = req->request.extlen;
    reqh.request.bodylen = reqh.request.keylen + reqh.request.extlen;   
    int header_size = sizeof(response_header);

    
    // Testing GET method
    buffer[0] = 0x80;buffer[1] = 0x00;buffer[2] = 0x00;buffer[3] = 0x05;
    buffer[4] = 0x00;buffer[5] = 0x00;buffer[6] = 0x00;buffer[7] = 0x00;
    buffer[8] = 0x00;buffer[9] = 0x00;buffer[10] = 0x00;buffer[11] = 0x05;
    buffer[12] = 0x00;buffer[13] = 0x00;buffer[14] = 0x00;buffer[15] = 0x00;
    buffer[16] = 0x00;buffer[17] = 0x00;buffer[18] = 0x00;buffer[19] = 0x00;
    buffer[20] = 0x00;buffer[21] = 0x00;buffer[22] = 0x00;buffer[23] = 0x00;

    buffer[24] = 0x48;buffer[25] = 0x65;buffer[26] = 0x6c;buffer[27] = 0x6c;
    buffer[28] = 0x6f;
   
    
    /*
    // Testing SET method
    buffer[0] = 0x80;buffer[1] = 0x01;buffer[2] = 0x00;buffer[3] = 0x05;
    buffer[4] = 0x04;buffer[5] = 0x00;buffer[6] = 0x00;buffer[7] = 0x00;
    buffer[8] = 0x00;buffer[9] = 0x00;buffer[10] = 0x00;buffer[11] = 0xE;
    buffer[12] = 0x00;buffer[13] = 0x00;buffer[14] = 0x00;buffer[15] = 0x00;
    buffer[16] = 0x00;buffer[17] = 0x00;buffer[18] = 0x00;buffer[19] = 0x00;
    buffer[20] = 0x00;buffer[21] = 0x00;buffer[22] = 0x00;buffer[23] = 0x00;
   
    buffer[24] = 0xde;buffer[25] = 0xad;buffer[26] = 0xbe;buffer[27] = 0xef;

    buffer[28] = 0x48;buffer[29] = 0x65;buffer[30] = 0x6c;buffer[31] = 0x6c;
    buffer[32] = 0x6f;buffer[33] = 0x57;buffer[34] = 0x6f;buffer[35] = 0x72;
    buffer[36] = 0x6c;buffer[37] = 0x64;
    */
    
    int tot_len = header_size + buffer[11];
    
    printf("The value of size of buffer sent to the server is %d  \n",(int) tot_len);

    //n = write(sockfd, buffer, strlen(buffer));
    n = write(sockfd, buffer, tot_len);

    if (n < 0) 
         error("ERROR writing to socket");
    
    
    bzero(buffer,256);
    
    n = read(sockfd, buffer, 255);
    
    printf("The value of size of response buffer read from the server  %d  \n",(int) n);

    if (n < 0) 
         error("ERROR reading from socket \n");
    
    printf("%s\n", buffer);
    
    for(int i = 0 ; i < n ; i++)
       printf("%x ", buffer[i]);

    printf("%s\n", buffer);

    close(sockfd);
    return 0;
    
}

