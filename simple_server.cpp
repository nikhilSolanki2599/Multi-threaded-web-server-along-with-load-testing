#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<signal.h>
#include <netinet/in.h>

#include <pthread.h>
#include "http_server.hh"
#define MAX_CLIENTS 10000
#define NUM_THREADS 200

pthread_t threads[NUM_THREADS];
int shared_buffer[MAX_CLIENTS];
pthread_mutex_t mutexQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condQueue = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
int active_clients =0;
int sockfd,fg;

void error(char *msg) 
{
  perror(msg);
  // exit(1);
}
void  INThandler(int sig)
{
fg=1;
  pthread_cond_broadcast(&condQueue);
  for(int i=0;i<NUM_THREADS;i++)
  {
    pthread_cancel(threads[i]);
    pthread_join(threads[i],NULL);
  }
  exit(0);
}
void *workerfunc(void *arg) {
   while(1){
    
    int temp;
    pthread_mutex_lock(&mutexQueue);
    while(active_clients == 0){
       pthread_cond_wait(&condQueue, &mutexQueue);
       if(fg==1)
      {
          pthread_mutex_unlock(&mutexQueue);
          pthread_exit(NULL);
      }
    }
    temp = shared_buffer[0];
    for(int i=0;i<active_clients;i++){
      shared_buffer[i] = shared_buffer[i+1];
    }
    active_clients--;
    pthread_cond_signal(&cond_full);
    pthread_mutex_unlock(&mutexQueue);
    char buffer[256];
    bzero(buffer, 256);
    int n = read(temp, buffer, 255);
    if (n <= 0){
        error("ERROR reading from socket");
        close(temp);
        continue;
    }
    HTTP_Response* response = handle_request(buffer);
    string x = response->get_string();
    n = write(temp, x.c_str(), x.length());
    if (n < 0)
      error("ERROR writing to socket");
    close(temp);
    delete response;
   }
 }
void *exec_work(int socket)
{
    signal(SIGINT, INThandler);
    pthread_mutex_lock(&mutexQueue);
    shared_buffer[active_clients++] = socket;
    pthread_cond_signal(&condQueue);
    pthread_mutex_unlock(&mutexQueue);
       
    return NULL;
}
int main(int argc, char *argv[]) {
  signal(SIGINT, INThandler);
  int  newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  listen(sockfd, 10000);
  clilen = sizeof(cli_addr);
  for(int i=0;i<NUM_THREADS;i++){
    pthread_create(&threads[i], NULL,workerfunc, NULL);
  }
  while(1)
  {
        
        pthread_mutex_lock(&mutexQueue);
        if(active_clients == MAX_CLIENTS)
        {
          pthread_cond_wait(&cond_full, &mutexQueue);
        }
        pthread_mutex_unlock(&mutexQueue);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
          error("ERROR on accept");
        exec_work(newsockfd);
    
  }
  return 0;
}