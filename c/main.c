#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include <pthread.h>
#include <regex.h>

#define PORT  1010 

char *buf;
pthread_t *threads;
extern volatile int busy;
extern volatile int stop;

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

  regex_t regex;
  char msgbuf[100];
  int r;

  //regex matches all /*.* files 
  r = regcomp(&regex, "^/.*[.].*", 0);
  if (r) {
      fprintf(stderr, "Could not compile regex\n");
      exit(1);
  }

  if (0 != strcmp (method, "GET"))
    return MHD_NO;

  if (strcmp(url, "/stop") == 0 ) {

    printf("%s\n", "arbeit macht frei!"); 
    stop = 1;
    char *res = "fertish";
    response = MHD_create_response_from_buffer (strlen(res), res, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;

  } else if (strcmp(url, "/out") == 0 ) {

    response = MHD_create_response_from_buffer (strlen(buf), buf, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;

  } else if (strcmp(url, "/") == 0 ) {
    FILE *f = fopen("../src/index.html", "r");
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 
    
    char *res = malloc(fsize + 1);
    fread(res, 1, fsize, f);
    fclose(f);
    
    res[fsize] = 0;

    response = MHD_create_response_from_buffer (strlen(res), res, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
  
    return ret;

  } else if (!regexec(&regex, url, 0, NULL, 0)) {
    //send all files from 'src' folder
    //
    char file[20];
    strcpy(file, "../src");
    strcat(file, url);
    
    FILE *f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 
    char *res = malloc(fsize + 1);
    
    fread(res, 1, fsize, f);
    fclose(f);
    res[fsize] = 0;
    response = MHD_create_response_from_buffer (strlen(res), res, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
  
    return ret;

  } else if (!strcmp(url, "/coordinates")) {
    
    printf("%s\n", "incoming root request..");
    const char* xstt = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "xstt"); 
    const char* ystt = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "ystt"); 
    const char* xend = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "xend"); 
    const char* yend = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "yend"); 
    const char* wi = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "wi"); 
    const char* he = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "he"); 
    const char* it = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "it"); 
    const char* bound = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "bound"); 
    const char* j = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "j"); 
    const char* jx = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "jx"); 
    const char* jy = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "jy"); 
    const char* pat = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "pat"); 
    
    int w = strtol(wi, NULL, 10);
    int h = strtol(he, NULL, 10);
    
    //reject if threads are busy
    if (busy == 0){
      buf = malloc (100 * w * h);
      stop = 1;
      sleep(0.5);
      stop = 0;
      threads = calcc(buf, xstt, ystt, xend, yend, it, wi, he, bound, j, jx, jy, pat);
    }else{
      printf("%s\n", "busy :/");
    }
    
    char *data = "/out";

    response = MHD_create_response_from_buffer (strlen(data), data, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
  
  }else{
    char *data = "/salam";

    response = MHD_create_response_from_buffer (strlen(data), data, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;

  }
  regfree(&regex);
}


int
main () {
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
  if (NULL == daemon)
    return 1;

  (void) getchar ();

  MHD_stop_daemon (daemon);

  return 0;
}
