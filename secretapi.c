#ifdef __linux__ 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <inttypes.h>
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 9988
#define DATALEN 150
#define HEADERLEN 10
#define MSGLEN (DATALEN+HEADERLEN)

#ifdef TARGET
#include "spi.h"
#endif

void diep(char *s)
{
    perror(s);
    exit(1);
}

void render(char* buf) {
int j, k;
char* obf;

    obf = &buf[HEADERLEN];

#ifdef TARGET
    // And send it on its merry way
    spi_send(obf, DATALEN); 
#endif
#ifdef DEBUG
    // This is where we get dirty with spi.h
    // For the moment, just show the values in the buffer.
    for (j=0, k=0; j < DATALEN; j++) {
        printf("%02x ", (uint8_t) obf[j]);
        if ((++k % 3) == 0) {
            printf("  ");
        }
        if ((k % 15) == 0) {
            printf("\n");
        }
    }
    printf("\n");
#endif
    return;
}

int main(void)
{
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    ssize_t num_datas;

   if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      diep("socket");

    memset((void *) &si_me, (int) 0, (size_t) sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (const struct sockaddr*) &si_me, sizeof(si_me))==-1)
        diep("bind");


#ifdef TARGET
    spi_open();
#endif

    //for (i=0; i<NPACK; i++) {
    while (1) {          // Basically we're gonna loop forever...
        num_datas = recvfrom(s, (void*) buf, BUFLEN, 0, (struct sockaddr*) &si_other, &slen);
      if (num_datas ==-1)
        diep("recvfrom()");
      if (num_datas != MSGLEN) {
       printf("Rejecting packet of length %d from %s:%d\nData: %s\n\n", (int) num_datas,
             inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);  
      } else {
#ifdef DEBUG
        printf("Received packet of length %d from %s:%d\n", (int) num_datas,
             inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
#endif
        render(buf);       // Render from here.
      }
    }

    close(s);
#ifdef TARGET
    spi_close();
#endif
    return 0;
}