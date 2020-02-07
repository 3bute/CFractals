#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "gmp_c.h"
#include <pthread.h>

#define PORT  1010 
#define NUM_THREADS 4 

char *buf;
pthread_t *threads;
int running = 0;

static int
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
  struct MHD_Response *response;
  int fd;
  int ret;
  struct stat sbuf;
  if (0 != strcmp (method, "GET"))
    return MHD_NO;

  if (strcmp(url, "/stop") == 0 ){

    printf("%s\n", "arbeit macht frei!"); 
    int i = 0;
    for ( i = 0; i < NUM_THREADS; i++) {
      pthread_cancel(threads[i]);
    }
    
    char *res = "fertish";
    response = MHD_create_response_from_buffer (strlen(res), res, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;

  }else if (strcmp(url, "/out") == 0 ){
    
    response = MHD_create_response_from_buffer (strlen(buf), buf, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;

  }else{

    printf("%s\n", "incoming root request..");
    const char* xstt = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "xstt"); 
    const char* ystt = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "ystt"); 
    const char* xend = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "xend"); 
    const char* yend = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "yend"); 
    const char* wi = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "wi"); 
    const char* he = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "he"); 
    const char* it = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "it"); 
    const char* bound = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "bound"); 
    const char* zoom = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "zoom"); 
    
    //allocate memory for the buffer,
    //that holds all deltas in json with corrsd. x and y,
    //additional storage is for separators.
    int w = strtol(wi, NULL, 10);
    int h = strtol(he, NULL, 10);
    buf = malloc(sizeof(char)*200*w*h);
    
    //try to stop them all
    if (running){
      int i = 0;
      for ( i = 0; i < NUM_THREADS; i++) {
        pthread_cancel(threads[i]);
      }
    }else{
      running = 1;
    }

    //start async calculations
    threads = calcc(buf, xstt, ystt, xend, yend, it, wi, he, bound, zoom);
    char *data = "/out";

    response = MHD_create_response_from_buffer (strlen(data), data, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
  }
}


int
main ()
{
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
  if (NULL == daemon)
    return 1;

  (void) getchar ();

  MHD_stop_daemon (daemon);

  return 0;
}
