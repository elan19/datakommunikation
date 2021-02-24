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

// Included to get the support library
#include "calcLib.h"

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass argument during compilation '-DDEBUG'
#define DEBUG
#define PROTOCOL "TEXT TCP 1.0\n\n"

using namespace std;

int main(int argc, char *argv[])
{

  if (argc != 2)
  {
    printf("Wrong format IP:PORT\n");
    exit(0);
  }

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter.

  /* Do magic */

  if (Desthost == NULL || Destport == NULL)
  {
    printf("Wrong format.\n");
    exit(0);
  }
  int port = atoi(Destport);

  int sockfd, connfd, len;
  struct sockaddr_in cli;

  addrinfo sa, *si, *p;
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_UNSPEC;
  sa.ai_socktype = SOCK_STREAM;

  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr, "%s\n", gai_strerror(rv));
    exit(0);
  }

  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("Error: Couldnt connect.\n");
      continue;
    }

    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      printf("Error: Couldnt bind!\n");
      close(sockfd);
      continue;
    }
    break;
  }

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
  if (p == NULL)
  {
    printf("NULL\n");
    exit(0);
  }

  freeaddrinfo(si);

  if (listen(sockfd, 5) == -1)
  {
    printf("Error: Listen failed!\n");
    exit(0);
  }

  len = sizeof(cli);
  bool clientIsActive = false;
  char buffer[128];
  char recvBuffer[128];
  double d1, d2, dAnsw, dRecvAnswer;
  int int1, int2, intAnsw, intRecvAnswer;
  char *arith;

  printf("Listening\n");
  initCalcLib();

  while (true)
  {
    if (clientIsActive == false)
    {
      if ((connfd = accept(sockfd, (struct sockaddr *)&cli, (socklen_t *)&len)) == -1)
      {
        printf("Couldnt accept anything, trying again!\n");
        continue;
      }
      else
      {
        clientIsActive = true;
        char buf[128] = PROTOCOL;
        arith = randomType();
        if (send(connfd, buf, strlen(buf), 0) == -1)
        {
          printf("Error: Send failed!\n");
          exit(0);
        }
      }
    }

    memset(buffer, 0, sizeof(buffer));
    memset(recvBuffer, 0, sizeof(recvBuffer));
    if (recv(connfd, recvBuffer, sizeof(recvBuffer), 0) == -1)
    {
      if(errno == EAGAIN)
      {
      printf("Error: Recieve timeout, sending error to client!\n");
      send(connfd, "ERROR TO\n", strlen("ERROR TO\n"), 0);
      }
      else{
        printf("Something went terrible wrong with the recieve!\n");
      }
      close(connfd);
      clientIsActive = false;
      continue;
    }

    if (strcmp(recvBuffer, "OK\n") == 0)
    {

      if (arith[0] == 'f')
      {
        d1 = randomFloat();
        d2 = randomFloat();
        if (strcmp(arith, "fadd") == 0)
        {
          dAnsw = d1 + d2;
        }
        else if (strcmp(arith, "fsub") == 0)
        {
          dAnsw = d1 - d2;
        }
        else if (strcmp(arith, "fmul") == 0)
        {
          dAnsw = d1 * d2;
        }
        else if (strcmp(arith, "fdiv") == 0)
        {
          dAnsw = d1 / d2;
        }
        sprintf(buffer, "%s %lf %lf\n", arith, d1, d2);
      }
      else
      {
        int1 = randomInt();
        int2 = randomInt();
        if (strcmp(arith, "add") == 0)
        {
          intAnsw = int1 + int2;
        }
        else if (strcmp(arith, "sub") == 0)
        {
          intAnsw = int1 - int2;
        }
        else if (strcmp(arith, "mul") == 0)
        {
          intAnsw = int1 * int2;
        }
        else if (strcmp(arith, "div") == 0)
        {
          intAnsw = int1 / int2;
        }
        sprintf(buffer, "%s %d %d\n", arith, int1, int2);
      }
    }
    else if (arith[0] == 'f')
    {
      sscanf(recvBuffer, "%lf", &dRecvAnswer);
      if (abs(dAnsw - dRecvAnswer) < 0.0001)
      {
        sprintf(buffer, "%s", "OK\n");
        printf("Client handled, the results match!\n");
        clientIsActive = false;
      }
      else
      {
        sprintf(buffer, "%s", "ERROR\n");
        printf("Client handled, the results did not match!\n");
        clientIsActive = false;
      }
    }
    else if (arith[0] != 'f')
    {
      sscanf(recvBuffer, "%d", &intRecvAnswer);
      if (intAnsw == intRecvAnswer)
      {
        sprintf(buffer, "%s", "OK\n");
        printf("Client handled, the results match!\n");
        clientIsActive = false;
      }
      else
      {
        sprintf(buffer, "%s", "ERROR\n");
        printf("Client handled, the results did not match!\n");
        clientIsActive = false;
      }
    }
    else
    {
      clientIsActive = false;
    }
    if (send(connfd, buffer, strlen(buffer), 0) == -1)
    {
      printf("Error: Send failed!\n");
      exit(0);
    }
  }

#ifdef DEBUG
  printf("Host %s, and port %d.\n", Desthost, port);
#endif

  close(sockfd);
  return 0;
}
