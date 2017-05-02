/*
 * File:     server.c
 * Date:     2011-04-15
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Print content of remote directory - server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>

// sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>

// regular expressions
#include <regex.h>

// directories
#include <dirent.h>

// protocol
#include "prot.h"


#define SOCKET_BUFFER 1024


/** Program error codes */
enum tecodes
{
    EOK = 0,         /**< Without error */
    EPARAM,          /**< Bad command line parameters */
    EPAR_PORT,       /**< Command line parameters: missing port number */
 
    ESOC_CREATE,     /**< Socekt Error: create */
    ESOC_RESOLVE,    /**< Socekt Error: resolving url */
    ESOC_CONNECT,    /**< Socekt Error: connect */
    ESOC_WRITE,      /**< Socekt Error: write */
    ESOC_READ,       /**< Socekt Error: read */
    ESOC_CLOSE,      /**< Socekt Error: close */

    EOUT_MEM,        /**< Out of memory */
    EBIND,           /**< Unable to bind */

    EUNKNOWN,        /**< Unknown error */
};

/** Error messages */
const char *ECODEMSG[] =
{
    /* EOK */
    "Everything OK.\n",
    /* EPARAM */
    "Error: Bad command line parameters!\n",
    /* EPAR_PORT */
    "Error: Missing port number!\n",

    /* ESOC_CREATE */
    "Socekt Error: create!\n",
    /* ESOC_RESOLVE */  
    "Socekt Error: resolving url!\n",
    /* ESOC_CONNECT */    
    "Socekt Error: connect!\n",
    /* ESOC_WRITE */     
    "Socekt Error: write!\n",
    /* ESOC_READ */    
    "Socekt Error: read!\n",
    /* ESOC_CLOSE */      
    "Socekt Error: close!\n",

    /* Out of memory */
    "Error: Out of memory. Sorry!\n",

    /* Unable to bind */
    "Error: Unable to bind to the given port!\n", 

    /* EUNKNOWN */
    "Error: Unknown!\n"
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
    int port;               /**< server port */
    int ecode;              /**< error code (enum tecodes) */
} tParams;

/**
 * Structure used to store request and response sent/received from server
 */
typedef struct message
{
    char *content;         /**< Received/sent message to server */
    int length;            /**< Length of message */
} tMessage;


/**
 * Prints error message to stderr
 * @param ecode error code
 */
void print_ecode(int ecode)
{
    if (ecode < EOK || ecode > EUNKNOWN)
    { 
        ecode = EUNKNOWN; 
    }

    fprintf(stderr, "%s", ECODEMSG[ecode]);
}

/**
 * Frees a pointer if it's not NULL
 * @param *p pointer to free
 */
void free2(void *p)
{
    if (p != NULL)
    {
        free(p);
    }
}

/**
 * Processes arguments of command line
 * @param argc Argument count
 * @param argv Array of string with arguments
 */
tParams get_params(int argc, char *argv[])
{
    tParams result = 
    {  // init structure
        .port = 0,
        .ecode = EOK
    };

    if (argc == 3 && strcmp(argv[1], "-p") == 0)
    {
        char *endptr;
        errno = 0;
        int val = strtod(argv[2], &endptr);

        if (errno != 0 || endptr == argv[2])
        {
            result.ecode = EPAR_PORT;
        }
        
        result.port = val;
    }
    else
    {
        result.ecode = EPARAM;
    }

    return result;
}

/**
 * Creates the response header
 * @param *response
 * @param code
 */
int create_response_head(tMessage *response, int code)
{
    int n = sprintf(response->content, dim_response_header_format, 
                                       dim_version, code, dim_del);
    response->length = n;
    return n;
}

/**
 * Creates the end of response
 * @param *response
 */
int create_response_tail(tMessage *response)
{
    strcat(response->content, dim_end);

    response->length += sizeof(dim_end);
    return response->length;
}

/**
 * Creates an error response
 * @param *response
 * @param code
 */
int create_error_response(tMessage *response, int code)
{
    create_response_head(response, code);    
    return create_response_tail(response);
}
 
/**
 * Reads the request from socket
 * @param *request
 * @param client_sockfd
 */
int read_request(tMessage *request, int client_sockfd)
{
    char *tmp;
    char *end = NULL;
    int result = 0;
    int size = 0;
    int chunk;
    request->length = 0;

    do
    {
        chunk = (size != 0) ? size : SOCKET_BUFFER;
        size += chunk;

        tmp = (char*)realloc(request->content, size * sizeof(char));

        if (tmp == NULL)
        {
            close(client_sockfd);
            return EOUT_MEM;
        }

        request->content = tmp;

        result = read(client_sockfd, request->content + request->length, chunk);
        
        if (result < 0)
        {
            close(client_sockfd);
            return ESOC_READ;
        }
        
        request->length += result;

        end = strstr(request->content, dim_end);
    }
    while (end == NULL && result >= chunk);

    return EOK;
}

/**
 * Creates the final response 
 * @param *request
 * @param *response
 */
int create_response(tMessage *request, tMessage *response)
{
    // check request
    char *str = request->content;
    char pattern[100];
    sprintf(pattern, dim_request_pattern, dim_request_kw, dim_version, dim_del, 
                                          dim_request_par, dim_end);

    regex_t re;
    regmatch_t pmatch[2];

    if (regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) 
    {
        return EUNKNOWN;
    }

    int stat = regexec(&re, str, 2, pmatch, 0);

    // allocate buffer for response
    response->content = (char*)malloc(SOCKET_BUFFER * sizeof(char));

    if (response->content == NULL)
    {
        return EOUT_MEM;
    }

    // can not understand request
    if (stat != 0)
    {
        create_error_response(response, DIM_ST_BAD_REQ);
        return EOK;
    }

    // understood the request, grab the path
    int n = pmatch[1].rm_eo - pmatch[1].rm_so;
    char *path = (char*)malloc((n + 1) * sizeof(char));
    strncpy(path, str + pmatch[1].rm_so, n);    

    regfree(&re);

    // open path
    errno = 0;
    DIR *dirp = opendir(path);

    if (dirp == NULL)
    {
        // something went wrong

        switch (errno)
        {
            case 13: create_error_response(response, DIM_ST_NOT_ENOUGH_RIGHT); break;
            case 20: create_error_response(response, DIM_ST_NOT_DIR ); break;
            case 2:  create_error_response(response, DIM_ST_PATH_NOT_FOUND); break;
            default: create_error_response(response, DIM_ST_UNKNOWN); break;
        }

        return EOK;
    }
    
    // everything fine, get directory contents
    create_response_head(response, DIM_ST_OK);

    char *w = response->content;
    char *tmp;
    int size = SOCKET_BUFFER;

    int del_len = sizeof(dim_del);

    int writed = 0;

    struct dirent *dp = readdir(dirp);

    while (dp != NULL)
    {
        if ((dp->d_namlen + del_len) >= (size - response->length))
        {
            size *= 2;

            if ((dp->d_namlen + del_len) >= (size - response->length))
            {
                continue;
            }

            tmp = (char*)realloc(response->content, size * sizeof(char));

            if (tmp == NULL)
            {
                closedir(dirp);
                return EOUT_MEM;
            }

            response->content = tmp;
            w = response->content;
        }

        writed = 0;

        if ((dp->d_type == DT_DIR || dp->d_type == DT_REG || dp->d_type == DT_LNK) 
             && (strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0)
           )
        {
            w = strcat(w, dp->d_name);
            response->length += dp->d_namlen;
            writed = 1;
        }

        dp = readdir(dirp);
        
        if (writed != 0 && dp != NULL)
        {
            w = strcat(w, dim_del);
            response->length += del_len;
        }  
    }

    closedir(dirp);

    create_response_tail(response);

    return EOK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Main program
 */
int main(int argc, char *argv[])
{
    // Process parameters
    // -------------------------------------------------------------------------

    tParams params = get_params(argc, argv);

    if (params.ecode != EOK)
    { // wrong parameters
        print_ecode(params.ecode);
        return EXIT_FAILURE;
    }

    // -------------------------------------------------------------------------
    

    // Main
    // -------------------------------------------------------------------------

    int server_sockfd;
    int client_sockfd;
    int server_len;
    socklen_t client_len;
    
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(params.port);
    server_len = sizeof(server_address);

    if (bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
    {
        print_ecode(EBIND);
        return EXIT_FAILURE;
    }

    // Create a connection queue, ignore child exit details and wait for clients
    listen(server_sockfd, 5);

    signal(SIGCHLD, SIG_IGN);

    while(1)
    {
        // Accept connection
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
            (struct sockaddr *)&server_address, &client_len);

        // Fork to create a process for this client
        // and perform a test to see whether we're the parent or the child
        if (fork() == 0)
        {
            // child
            int exit_code = EXIT_SUCCESS;
            tMessage request;
            tMessage response;
            request.content = NULL;
            response.content = NULL;

            int result = read_request(&request, client_sockfd);
            
            if (result == EOK)
            {
                result = create_response(&request, &response);
            }

            if (result == EOK)
            {
                if (write(client_sockfd, response.content, response.length) < 0)
                {
                    result = ESOC_WRITE;
                }
            }

            free2(request.content);
            free2(response.content);

            if (result != EOK)
            {
                print_ecode(result);
                exit_code = EXIT_FAILURE;
            }

            if (close(client_sockfd) < 0)
            {
                print_ecode(ESOC_CLOSE);
                exit_code = EXIT_FAILURE;
            }

            exit(exit_code);
        }
        else
        {
            // parent
            close(client_sockfd);
        }
    }

    return EXIT_SUCCESS;
}

/* end of server.c */
