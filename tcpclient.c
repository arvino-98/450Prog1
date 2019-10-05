/* tcp_ client.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2019 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024
#define HOST_PORT 65309
const char *HOST_NAME = "cisc450.cis.udel.edu";
const char *OUT_FILE = "out.txt";

/* Header structure */
typedef struct {
  uint16_t count;
  uint16_t packet_sequence_number;
}Header;

int main(void) {

   int sock_client;  /* Socket used by client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   char filename[STRING_SIZE];  /* send filename */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */

   char line[STRING_SIZE];
   memset(line, 0, sizeof(line)); // make sure line is clean of garbage

   FILE *f_out = fopen(OUT_FILE, "w");

   int total_packets = 0;
   int total_data_bytes = 0;


   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Client: can't open stream socket");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port
            (in which case, do it the same way as in udpclient.c).
            The local address initialization and binding is done automatically
            when the connect function is called later, if the socket has not
            already been bound. */


   /* initialize server address information */
   strcpy(server_hostname, HOST_NAME);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname");
      close(sock_client);
      exit(1);
   }
   server_port = HOST_PORT;

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

    /* connect to the server */

   if (connect(sock_client, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Client: can't connect to server");
      close(sock_client);
      exit(1);
   }

   /* prompt user for filename */
   printf("Please input a filename:\n");
   scanf("%s", filename);
   msg_len = strlen(filename) + 1;

   /* send filename header */
   Header *filename_h = malloc(sizeof(Header));
   filename_h->count = htons(msg_len);
   filename_h->packet_sequence_number = htons(0);
   bytes_sent = send(sock_client, filename_h, sizeof(filename_h), 0);
    /* send filename */
   bytes_sent = send(sock_client, filename, msg_len, 0);

   /* get response from server */
   Header *h = malloc(sizeof(Header));
   for(;;) {
     bytes_recd = recv(sock_client, h, sizeof(h), 0);
     uint16_t count = count = ntohs(h->count);
     uint16_t psn = ntohs(h->packet_sequence_number);
     total_packets++;
     total_data_bytes += count;

     /* Break when count is 0 (meaning we received EOT packet) */
     if (count == 0){
       printf("End of Transmission Packet with sequence number %hu"
              " received with %hu data bytes\n", psn, count);
       break;
     }

     recv(sock_client, line, count, 0);
     printf("Packet %hu recevied with %hu data bytes\n", psn, count);

     fputs(line, f_out);
     memset(line, 0, sizeof(line));

   }

   /* free allocated memory and close connection */
   free(filename_h);
   free(h);
   printf("Number of packets received: %d\n", total_packets);
   printf("Total number of data bytes received: %d\n", total_data_bytes);
   close(sock_client);
}
