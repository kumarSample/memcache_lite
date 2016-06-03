/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: kumars
 *
 * Created on June 1, 2016, 12:48 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

#include <time.h>
#include <assert.h>
#include <limits.h>
#include <sysexits.h>
#include <stddef.h>

#include <pwd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h> 

#include <boost/array.hpp>
#include <map>


#include "ProtDefs.h"


using namespace std;

std::map<std::string, std::string> memCacheMap;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     
     char buffer[256];
     char buffer_out[256];
     //char buffer_data[256];
     char *buffer_response,*buffer_body,*buffer_body_out;
     
     struct sockaddr_in serv_addr, cli_addr;
     
     int n,buffer_len,buffer_key_value_len,valid_prot =0;
     char *buffer_ptr,*buffer_key,*buffer_key_value;
     int valid_prot_get,valid_prot_set;

     request_header  reqh;
     response_header resh;
     
     pthread_mutex_t mp;    
     int ret;
     int i;
     
     
     memset(&resh, 0, sizeof(resh));
     memset(&reqh, 0, sizeof(reqh));
     memset(&buffer_out, 0, 256);

     
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
 
 
     char test[256];
   
     // For testing purposes
     //memCacheMap["Hello"] =  "world";
     
     // create a socket
     // socket(int domain, int type, int protocol)
     
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     
     if (sockfd < 0) 
        error("ERROR opening socket");

     // clear address structure
     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = atoi(argv[1]);

     /* setup the host_addr structure for use in bind call */
     // server byte order
     serv_addr.sin_family = AF_INET;  

     // automatically be filled with current host's IP address
     serv_addr.sin_addr.s_addr = INADDR_ANY;  

     // convert short integer value for port must be converted into network byte order
     serv_addr.sin_port = htons(portno);
     

     // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
     // bind() passes file descriptor, the address structure, 
     // and the length of the address structure
     // This bind() call will bind  the socket to the current IP address on port, portno
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     while(1)
     {
     // This listen() call tells the socket to listen to the incoming connections.
     // The listen() function places all incoming connection into a backlog queue
     // until accept() call accepts the connection.
     // Here, we set the maximum size for the backlog queue to 5.
     listen(sockfd,5);

     // The accept() call actually accepts an incoming connection
     clilen = sizeof(cli_addr);

     // This accept() function will write the connecting client's address info 
     // into the the address structure and the size of that structure is clilen.
     // The accept() returns a new socket file descriptor for the accepted connection.
     // So, the original socket file descriptor can continue to be used 
     // for accepting new connections while the new socker file descriptor is used for
     // communicating with the connected client.
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     printf("server: got connection from %s port %d\n",
            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
     
     bzero(buffer,256);
     //bzero(buffer_data,256);
     
     int header_size = sizeof(response_header);
     printf("The value of response header is %d \n", header_size); // debugging

     n = read(newsockfd,buffer,header_size);
     
     printf("The number of bytes read first time  %d \n",n);
     
     response_header* reshp;
     reshp = (response_header*)buffer_out;
     buffer_ptr = buffer_out;
    
     reshp->response.magic  = RESPONSE;   
     reshp->response.opcode = CMD_GET; 
     reshp->response.keylen =  0;
     reshp->response.bodylen = 0; 
     reshp->response.datatype = 0;   

     //resh->response.bodylen = htonl(req->request.bodylen +resh->response.keylen);    
     reshp->response.extlen = 0;   
     reshp->response.status = RESPONSE_SUCCESS;
     reshp->response.opaque = 0;
     reshp->response.cas = 0;   
     

     if(n < header_size )
     {
         printf(" ERROR reading header less than header size \n");
         reshp->response.status = RESPONSE_EINVAL;
         send(newsockfd, buffer_out, header_size, 0);
         
         ret = pthread_mutex_unlock(&mp);
         printf("Value of pthread_mutex_unlock ret is %d \n", ret);
         continue; 
     }
     
     
     request_header* req;
     req = (request_header*)buffer;
     //buffer_ptr = buffer;
             
     reqh.request.keylen = ntohs(req->request.keylen);
     reqh.request.bodylen = ntohl(req->request.bodylen);    
     reqh.request.opcode = req->request.opcode;
     reqh.request.extlen = req->request.extlen;
     
     printf("The reqh.request.extlen  %d \n",reqh.request.extlen);
 
          
     valid_prot_get = 0;
     valid_prot_set = 0;
    
    
    if(reqh.request.opcode == CMD_GET) 
        valid_prot_get = 1;
    else if(reqh.request.opcode == CMD_SET) 
        valid_prot_set = 1;   
    else
    {
         printf(" ERROR  memCache protocol not supported other than GET and SET \n");
         reshp->response.status = RESPONSE_EINVAL;
         send(newsockfd, buffer_out, header_size, 0);
         ret = pthread_mutex_unlock(&mp);
         printf("Value of pthread_mutex_unlock ret is %d \n", ret);
         continue;         
    }
        
        
    if(valid_prot_set)   // SET method
    {
        printf(" SET method  \n");
       
        ret = pthread_mutex_lock(&mp);
        
        printf("Value of pthread_mutex_lock ret is %d \n", ret);
        
        memset(&reshp, 0, sizeof(reshp));

        reshp = (response_header*)buffer_out;

        reshp->response.magic  = RESPONSE;   
        reshp->response.opcode = CMD_SET; 
        reshp->response.keylen =  0;
        reshp->response.bodylen = 0; 
        reshp->response.datatype = 0;   
        
        //resh->response.bodylen = htonl(req->request.bodylen +resh->response.keylen);    
        reshp->response.extlen = 0;   
        reshp->response.status = RESPONSE_SUCCESS;
        reshp->response.opaque = 0;
        reshp->response.cas = 0;   
        
        
        
        buffer_body = (char *)malloc(reqh.request.bodylen);
        
        if(buffer_body)
          n = read(newsockfd,buffer_body,reqh.request.bodylen);
        else
        {
          printf("Memory error  \n");
          return 0;
        }
        
        
      
        
        buffer_ptr = buffer_body;

        printf("The number of bytes read second time  %d \n",n);
        printf("The value of key len is  %d \n",reqh.request.keylen);
        printf("The value of extlen is  %d \n",reqh.request.extlen);


        buffer_key = (char *)malloc(reqh.request.keylen);
        
        if(buffer_key)
           buffer_ptr = buffer_ptr + reqh.request.extlen;
        else
        {
           printf("Memory error  \n");
           return 0;
        }
        
        if(buffer_ptr)
            strncpy(buffer_key,buffer_ptr,reqh.request.keylen);
        else
        {
           printf("Memory error  \n");
           return 0;
        }
        
        printf("The value of key  is  %s \n",buffer_key);
              
        // Check if key already exists in the cache if so return error
        
        if(memCacheMap.count(buffer_key) == 1)
        {
           printf("The value of buffer_key %s already exists \n\n",buffer_key);
           reshp->response.status = RESPONSE_KEY_EEXISTS;
           send(newsockfd, buffer_out, header_size, 0);
           ret = pthread_mutex_unlock(&mp);
           printf("Value of pthread_mutex_unlock ret is %d \n", ret);
           continue;
        }
      

        buffer_key_value_len = reqh.request.bodylen-reqh.request.keylen-reqh.request.extlen;
        
        printf("The value of buffer_key_value_len is  %d \n",buffer_key_value_len);

        buffer_key_value = (char *)malloc(buffer_key_value_len);

            
        buffer_ptr = buffer_ptr + reqh.request.keylen;
        
        if(buffer_key_value)
            strncpy(buffer_key_value,buffer_ptr,buffer_key_value_len);
        else
        {
            printf("Memory error  \n");
            return 0;
        }

        printf("The value of keyvalue  %s \n",buffer_key_value);
        
        
        memCacheMap[buffer_key] =  buffer_key_value;
        
        std::cout << "Added Value to memCache  " << memCacheMap[buffer_key] << '\n';
        
        // Send SUCCESS response to the client
        
        send(newsockfd, buffer_out, header_size, 0);  
        
        ret = pthread_mutex_unlock(&mp);
        
        printf("Value of pthread_mutex_unlock ret is %d \n", ret);

    }    
    else if(valid_prot_get)
    {  
     
        ret = pthread_mutex_lock(&mp);
        printf("Value of pthread_mutex_lock ret is %d \n", ret);

        printf("The GET method  \n");

        buffer_body = (char *)malloc(reqh.request.bodylen);
        
        buffer_ptr = buffer_body;


        if(buffer_body)
            n = read(newsockfd,buffer_body,reqh.request.bodylen);
        else
        {
            printf("Memory error  \n");
            return 0;
        }
        
        printf("The number of bytes read second time  %d \n",n);
        
        
        if(n != reqh.request.bodylen)
        {
            printf("Memory error  \n");
            return 0;
        }
        
        
        // Moving past the extras 
        printf("The value of reqh.request.extlen  %d \n",reqh.request.extlen);
        buffer_ptr = buffer_ptr + reqh.request.extlen;
        
        buffer_key = (char *)malloc(reqh.request.keylen);


        if(buffer_key)
            strncpy(buffer_key,buffer_ptr,reqh.request.keylen);
        else
        {
            printf("Memory error  \n");
            return 0;
        }

   
        printf("The value of key  %s \n",buffer_key);
               
        
        std::string tmp = memCacheMap[buffer_key];
        
        // check if the key does not exist in the memcache
        
        if(tmp.empty())
        {
            memset(&reshp, 0, sizeof(reshp));
            printf("The key does not exist \n");
            reshp = (response_header*)buffer_out;

            
            reshp->response.magic  = RESPONSE;   
            reshp->response.opcode = CMD_GET; 
            reshp->response.keylen =  0;
            reshp->response.bodylen = 0; 
            reshp->response.datatype = 0;   

            //resh->response.bodylen = htonl(req->request.bodylen +resh->response.keylen);    
            reshp->response.extlen = 0;   
            reshp->response.status = RESPONSE_KEY_ENOENT;
            reshp->response.opaque = 0;
            reshp->response.cas = 0;   
            
            send(newsockfd, buffer_out, header_size, 0);
  
            ret = pthread_mutex_unlock(&mp);
            printf("Value of pthread_mutex_unlock ret is %d \n", ret);
            continue;

        }
                
        //buffer_key_value =  (char *) memCacheMap[buffer_key];
        
        buffer_key_value = (char *)tmp.c_str();
        
        printf(" Got Key Value %s \n",buffer_key_value) ;      
        printf("The value of buffer_key_value  %s  len  %d \n",buffer_key_value,(int) strlen(buffer_key_value));
        
        buffer_key_value_len = strlen(buffer_key_value);
        int len_response = header_size+ buffer_key_value_len + 4;  // 4 extras bytes
    
       
        printf("The value of len_response  %d \n",len_response);
       
        buffer_body_out = (char *)malloc(len_response);
        
        if(buffer_body_out == NULL)
        {
            printf("Memory error  \n");
            return 0;
        }

        reshp = (response_header*)buffer_body_out;

        buffer_ptr = buffer_body_out;
                
        reshp->response.magic  = RESPONSE;   
        reshp->response.opcode = CMD_GET; 
        reshp->response.keylen =  0;
        reshp->response.extlen =  htonl(4); 
        reshp->response.bodylen = htonl(buffer_key_value_len + reshp->response.extlen); 

        reshp->response.datatype = 0;   

        //resh->response.bodylen = htonl(req->request.bodylen +resh->response.keylen);    
        reshp->response.extlen = 0;   
        reshp->response.status = RESPONSE_SUCCESS;
        reshp->response.opaque = 0;
        reshp->response.cas = 0;

        //buffer_ptr = buffer_ptr + reqh.request.keylen;

                
        buffer_ptr = buffer_ptr + header_size;
        strncpy(buffer_ptr,"dead",reshp->response.extlen);  // just add extra 4 bytes as in the spec
    
        buffer_ptr = buffer_ptr + reshp->response.extlen;  // 4 extras bytes 
        strncpy(buffer_ptr,buffer_key_value,buffer_key_value_len);   
    
        printf("The value of buffer_out  %d \n",(int) strlen(buffer_out));
    
        int out_size = header_size + reshp->response.extlen + buffer_key_value_len;
    
        printf("The value of out_size  %d \n",out_size);

        send(newsockfd, buffer_body_out, out_size, 0);
        
      
        ret = pthread_mutex_unlock(&mp);
        
        printf("Value of pthread_mutex_unlock ret is %d \n", ret);

  
    }

     if (n < 0) error("ERROR reading from socket");
     //printf("Here is the message: %x  size of buffer %d \n",buffer[0],(int) strlen(buffer));
    }
     


     if(buffer_key)
         free(buffer_key);
     
     if(buffer_key_value)
         free(buffer_key_value);
     
     if(buffer_body)
         free(buffer_body);
 
     if(buffer_body_out)
         free(buffer_body_out);     
     close(newsockfd);
     close(sockfd);

     return 0; 
}

