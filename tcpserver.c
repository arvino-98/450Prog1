/* tcpserver.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2019 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 80
/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */
#define SERV_TCP_PORT 65309

/* Header structure */
typedef struct {
  uint16_t count;
  uint16_t packet_sequence_number;
}Header;

int main(void) {

   int sock_server;  /* Socket on which server listens to clients */
   int sock_connection;  /* Socket on which server exchanges data with client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned int server_addr_len;  /* Length of server address structure */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char filename[STRING_SIZE];  /* receive fielname */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
   FILE *fp;
   char *line = NULL;
   size_t len = 0;
   ssize_t read;

   int total_packets = 0;
   int total_data_bytes = 0;

   /* open a socket */
   if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Server: can't open stream socket");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_TCP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address");
      close(sock_server);
      exit(1);
   }

   /* listen for incoming requests from clients */

   if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
      perror("Server: error on listen"); /* requests that will be queued */
      close(sock_server);
      exit(1);
   }
   printf("I am here to listen ... on port %hu\n\n", server_port);

   client_addr_len = sizeof (client_addr);

   /* wait for incoming connection requests in an indefinite loop */
   for (;;) {
      sock_connection = accept(sock_server, (struct sockaddr *) &client_addr,
                                         &client_addr_len);
                     /* The accept function blocks the server until a
                        connection request comes from a client */
      if (sock_connection < 0) {
         perror("Server: accept() error\n");
         close(sock_server);
         exit(1);
      }

      // receive the filename header
      Header *filename_h = malloc(sizeof(Header));
      bytes_recd = recv(sock_connection, filename_h, sizeof(filename_h), 0);
      /* receive the filename */
      bytes_recd = recv(sock_connection, filename, STRING_SIZE, 0);

      if (bytes_recd > 0){
         printf("Received Filename is:\n");
         printf("%s\n", filename);

         fp = fopen(filename, "r");
         if (fp == NULL){
           printf ("Error creating file\n");
           close(sock_connection);
           return 1;
         }

        /* Read data from file and transmit */
        Header *h = malloc(sizeof(Header));
        unsigned short int psn = 1;
        while ((read = getline(&line, &len, fp)) != -1){
          h->count = htons(strlen(line));
          h->packet_sequence_number = htons(psn);
          bytes_sent = send(sock_connection, h, sizeof(h), 0);
          total_packets++;
          total_data_bytes += strlen(line);

          send(sock_connection, line, strlen(line), 0);
          printf("Packet %hu transmitted with %ld data bytes\n", psn, strlen(line));

          memset(line, 0, sizeof(line));
          psn += 1;
        }

        /* send EOT packet */
        h->count = htons(strlen(line));
        h->packet_sequence_number = htons(psn);
        bytes_sent = send(sock_connection, h, sizeof(h), 0);
        printf("End of Transmission Packet with sequence number %hu"
               " transmitted with %ld data bytes\n", psn, strlen(line));
        total_packets++;

        /* free allocated memory, close connection, and exit */
        free(filename_h);
        free(h);
        printf("Number of packets transmitted: %d\n", total_packets);
        printf("Total number of data bytes transmitted: %d\n", total_data_bytes);
        close(sock_connection);
        exit(0);
      }
   }
}
