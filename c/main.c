
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

#define PORT  1010 
static char *buf;
static int busy;

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


  if (strcmp(url, "/out") == 0 ){
    ////ignore when busy
    //if (busy){
    //  char *res = "busy";
    //  response = MHD_create_response_from_buffer (strlen(res), res, MHD_RESPMEM_PERSISTENT);
    //  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    //  MHD_destroy_response (response);
    //  return ret;
    //}
    
    printf("%s\n", "incoming request, checking buffer..");
    printf("%s\n", &buf[0]); 

    response = MHD_create_response_from_buffer (strlen(buf), buf, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

  }else{

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
    //that holds all delta values pixelwise,
    //additional storage is for separators.
    int w = strtol(wi, NULL, 10);
    int h = strtol(he, NULL, 10);
    buf = malloc(sizeof(char) * w * h * 2);
    buf[0] = "s";
    printf("%s\n", &buf[0]); 
    //start async calculations
    calcc(buf, xstt, ystt, xend, yend, it, wi, he, bound, zoom);
    char *data = "/out";

    response = MHD_create_response_from_buffer (strlen(data), data, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
  }
  return ret;
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
