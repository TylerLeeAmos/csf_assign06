// Tyler Amos, Justin Canedy
// tamos5@jhu.edu, jcanedy1@jhu.edu
#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"
#include "signal.h"
#include <pthread.h>

#define LINEBUF_SIZE 1024


/*
 * Data structure representing a client connection.
 */
struct ConnInfo {
  int clientfd;
  struct Calc *calc;

};


int chat_with_client(struct Calc *calc, int infd, int outfd) {
  rio_t in;
  char linebuf[LINEBUF_SIZE];

  /* wrap standard input (which is file descriptor 0) */
  rio_readinitb(&in, infd);

  /*
   * Read lines of input, evaluate them as calculator expressions,
   * and (if evaluation was successful) print the result of each
   * expression.  Quit when "quit" command is received.
   */
  int done = 0;
  while (!done) {
    ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
    
    if (n <= 0) {
      /* error or end of input */
      done = 1;
    
    } else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0 || linebuf[0] == 29) {
        /* quit command */
      done = 1;
     
    } else if(strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
        return 1;
   
    } else {
        /* process input line */
	int result;
	
	if (calc_eval(calc, linebuf, &result) == 0) {
	  /* expression couldn't be evaluated */
	  rio_writen(outfd, "Error\n", 6);
	
	} else {
	  /* output result */
	  int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
	
	  if (len < LINEBUF_SIZE) {
	    rio_writen(outfd, linebuf, len);
	  }
        }
      }
    }
  return 0;
}


void *worker(void *arg) {
  struct ConnInfo *info = arg;

   // set thread as detached
   pthread_detach(pthread_self());

   // handle client request
   chat_with_client(info->calc, info->clientfd, info->clientfd);
   // This might be where the shutdown request extra credit is? 
   close(info->clientfd);
   free(info);

   return NULL;
}



int main(int argc, char **argv) {
  if (argc != 2) {
    perror("Usage: webserver <port>");
  }
  const char *port = argv[1];
  int serverfd = open_listenfd((char*) port);

  if (serverfd < 0) { 
    perror("Couldn't open server socket");
  }

  struct Calc *calc = calc_create();
 
  while(1) {
    int clientfd = Accept(serverfd, NULL, NULL);
    
    if (clientfd < 0) { 
      perror("Error accepting client connection"); 
    }
    
    // create ConnInfo
    struct ConnInfo *info = malloc(sizeof(struct ConnInfo));
    info->calc = calc;
    info->clientfd = clientfd;

    // start new threat for client connection
    pthread_t thr_id;
    if (pthread_create(&thr_id, NULL, worker, info) != 0) {
      perror("pthread_create failed");
    }
  }
  return 0;
}

