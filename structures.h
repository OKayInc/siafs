#ifndef SIAFS_STRUCTURES_H
#define SIAFS_STRUCTURES_H

typedef struct{
    char *url;
    char *scheme;
    char *host;
    char *user;
    char *password;
    char *port_s;
    unsigned int port;
    char *bucket;
    char *unauthenticated_url;
    short verbose;
    unsigned int maxhandle;
}siafs_opt_t;

#endif
