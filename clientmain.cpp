#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
/* You will to add includes here */

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass 
#define DEBUG
#define PROTOCOL "TEXT TCP 1.0\n"


// Included to get the support library
#include "calcLib.h"

int main(int argc, char *argv[])
{

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */

  if(argc != 2)
  {
    printf("Wrong format IP:PORT\n");
    exit(0);
  }

  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);

  if(Desthost == NULL || Destport == NULL)
  {
    printf("Wrong format.\n");
    exit(0);
  }

  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 

  /* Do magic */

  int port=atoi(Destport);

  addrinfo sa, *si, *p;
  sa.ai_family = AF_INET;
  sa.ai_socktype = SOCK_STREAM;
  if(int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr,"%s\n", gai_strerror(rv));
    exit(0);
  }

  int sockfd;

  for(p = si; p != NULL; p = p->ai_next)
  {
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      continue;
    }

    if((connect(sockfd, p->ai_addr, p->ai_addrlen) == -1))
    {
      close(sockfd);
      printf("Error: Couldnt connect.\n");
      exit(0);
    }
    break;
  }


  if(p == NULL)
  {
    printf("NULL\n");
    exit(0);
  }

  freeaddrinfo(si);

  char buf[128];
  int bytes;


  if((bytes = recv(sockfd, buf, sizeof(buf), 0)) == -1)
  {
   printf("%s\n",strerror(errno));
    close(sockfd);
    exit(0);
  }

  printf("%s\n", buf);

  if(strstr(buf, PROTOCOL) == NULL)
  {
    printf("Wrong protocol.\n");
    close(sockfd);
    exit(0);
  }

  printf("Accepted protocol\n");

  if(send(sockfd, "OK\n", strlen("OK\n"),0) == -1)
  {
    printf("Error: Couldnt send\n");
    close(sockfd);
    exit(0);
  }

  memset(buf,0,128);

  if((bytes = recv(sockfd, buf, sizeof(buf), 0)) == -1)
  {
    printf("%s\n",strerror(errno));
    close(sockfd);
    exit(0);
  }

  printf("%s\n", buf);

  char operation[5];

  if(buf[0] == 'f')
  {
    double value1 = 0;
    double value2 = 0;
    double total = 0;

    sscanf(buf, "%s %lf %lf", operation, &value1, &value2);

    if(strstr(operation, "fadd"))
    {
      total = value1 + value2;
    }
    else if(strstr(operation, "fdiv"))
    {
      total = value1 / value2;
    }
    else if(strstr(operation, "fmul"))
    {
      total = value1 * value2;
    }
    else
    {
      total = value1 - value2;
    }

    printf("%lf\n", total);

    char answ[5];
    sprintf(answ, "%lf\n", total);

    if(send(sockfd, answ, strlen(answ),0) == -1) 
  {
    printf("Error: Couldnt send\n");
    close(sockfd);
    exit(0);
  }

  }
  else
  {
    int value1 = 0;
    int value2 = 0;
    int total = 0;

    sscanf(buf, "%s %d %d", operation, &value1, &value2);

    if(strstr(operation, "add"))
    {
      total = value1 + value2;
    }
    else if(strstr(operation, "div"))
    {
      total = value1 / value2;
    }
    else if(strstr(operation, "mul"))
    {
      total = value1 * value2;
    }
    else
    {
      total = value1 - value2;
    }

    printf("%d\n", total);

    char answ[3];
    sprintf(answ, "%d\n", total);

    if(send(sockfd, answ, strlen(answ),0) == -1)  
    {
      printf("Error: Couldnt send\n");
      close(sockfd);
      exit(0);
    }

  }

  memset(buf,0,128);

  if((bytes = recv(sockfd, buf, sizeof(buf), 0)) == -1)
  {
    printf("%s\n",strerror(errno));
    close(sockfd);
    exit(0);
  }

  printf("%s\n", buf);


  #ifdef DEBUG 
    printf("Host %s, and port %d.\n",Desthost,port);
  #endif

    close(sockfd);
    return 0;
}
