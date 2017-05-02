/*
 * File:     client.c
 * Date:     2011-04-15
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Print content of remote directory - client
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

// sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// regular expressions
#include <regex.h>

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

    /* EUNKNOWN */
    "Error: Unknown!\n"
};

/** Response status codes */
enum tscodes
{
    SOK = 0,           /**< OK */
    SBAD,              /**< Bad answer */
    S20,               /**< Path not found */
    S21,               /**< Not directory */
    S22,               /**< Not enough rights */
    S25,               /**< Unknown directory error */
    S30,               /**< Bad request */
    SXX,               /**< Unknown status code */
};

/** Status messages */
const char *SCODEMSG[] =
{
    /* SOK */
    "Everything is fine.\n",
    /* SBAD */
    "Error: Can't understand answer from server!\n",
    /* S20 */
    "Error: Path not found (status code is 20)!\n",
    /* S21 */
    "Error: Not a directory (status code is 21)!\n",
    /* S22 */
    "Error: Not enough rights (status code is 22)!\n",
    /* S25 */
    "Error: Unknown directory error (status code is 25)!\n",
    /* S30 */
    "Error: Bad request (status code is 30)!\n",
    /* SXX */
    "Error: Unknown status code!\n"
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
    char *url;              /**< server url */
    int port;               /**< server port */
    char *path;             /**< path on server */
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
 * Prints response status message to stderr
 * @param scode status code
 */
void print_scode(int scode)
{
    if (scode < SOK || scode > SXX)
    { 
        scode = SXX; 
    }

    fprintf(stderr, "%s", SCODEMSG[scode]);
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
        .url = NULL,
        .port = 0,
        .path = NULL,
        .ecode = EOK
    };

    if (argc == 3)
    {
        result.url = argv[1];
        result.path = argv[2];
        int ok = 0;
        char port_buffer[10];

        int n = strlen(argv[1]);
        int j = 0;
        for (int i = 0; i < n; i++)
        {
            if (ok == 1)
            {
                if (!isdigit(argv[1][i]))
                {
                    result.ecode = EPAR_PORT;
                    break;
                }
                
                port_buffer[j] = argv[1][i];
                j += 1;
            }

            if (ok != 1 && argv[1][i] == ':')
            {
                argv[1][i] = '\0';
                ok = 1;
            }
        }

        port_buffer[j] = '\0';

        result.port = atoi(port_buffer);

        if (ok != 1)
        {
            result.ecode = EPAR_PORT;
        }
    }
    else
    {
        result.ecode = EPARAM;
    }

    return result;
}

 
/**
 * Process request, open socket, send data, receive data
 * @param *hostName
 * @param port
 * @param *req
 * @param *res
 */
int process_request(char *hostName, int port, tMessage *req, tMessage *res)
{
    int s = 0;

    struct sockaddr_in sin; 
    struct hostent *hptr;

    // create socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {     
        return ESOC_CREATE;
    }

    // set protocol family to Internet
    sin.sin_family = PF_INET;

    // set port number
    sin.sin_port = htons(port);  

    if ((hptr = gethostbyname(hostName)) == NULL)
    {
        return ESOC_RESOLVE;
    }

    memcpy(&(sin.sin_addr), hptr->h_addr, hptr->h_length);

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        close(s);
        return ESOC_CONNECT;
    }

    // send message to server
    if (write(s, req->content, req->length) < 0) 
    {    
        close(s);
        return ESOC_WRITE;
    }

    // read message from server
    char *tmp;
    char *end = NULL;
    int result = 0;
    int size = 0;
    int chunk = 0;
    res->length = 0;

    do
    {
        if (res->length >= size)
        {
            chunk = (size != 0) ? size : SOCKET_BUFFER;
            size += chunk;

            tmp = (char*)realloc(res->content, size * sizeof(char));

            if (tmp == NULL)
            {
                close(s);
                return EOUT_MEM;
            }

            res->content = tmp;
        }

        result = read(s, res->content + res->length, chunk);
        
        if (result < 0)
        {
            close(s);
            return ESOC_READ;
        }
        
        res->length += result;
        chunk -= result;

        end = strstr(res->content, dim_end);
    }
    while (end == NULL && result != 0);

    // close connection, clean up socket
    if (close(s) < 0) 
    { 
        return ESOC_CLOSE;
    }

    return EOK;
}

/**
 * Create request that will be sent to server
 * @param *request
 */
int create_request(tMessage *request, char *path)
{
    request->content = (char*)malloc((strlen(path) + 100) * sizeof(char));

    if (request->content == NULL)
    {
        return EOUT_MEM;
    }

    sprintf(request->content, dim_request_format, dim_request_kw, dim_version,
                              dim_del, dim_request_par, path, dim_end);

    request->length = strlen(request->content);
    
    return EOK;
}

/**
 * Prints results to stdout, if error has found in response, 
 * then print error message to stderr
 * @param *response
 */
int print_results(tMessage *response)
{
    char *str = response->content;
    char pattern[100];
    sprintf(pattern, dim_response_pattern, dim_version, dim_del, dim_end);

    regex_t re;
    regmatch_t pmatch[3];

    if (regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) 
    {
        return EUNKNOWN;
    }

    int stat = regexec(&re, str, 3, pmatch, 0);

    if (stat != 0)
    {
        print_scode(SBAD);
        return EOK;
    }

    char statBuffer[10];
    strncpy(statBuffer, str + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
    statBuffer[pmatch[1].rm_eo - pmatch[1].rm_so] = '\0';

    int statusCode = atoi(statBuffer);

    if (statusCode == DIM_ST_OK)
    {
        str[pmatch[2].rm_eo] = '\0';
        char *s = str + pmatch[2].rm_so;
        char *e = strstr(s, dim_del);

        while (*s != '\0')
        {
            if (e == NULL)
            {
                e = str + pmatch[2].rm_eo;
            }

            while (s != e)
            {
                putchar(*s);
                s += 1;
            }

            if (pmatch[2].rm_so != pmatch[2].rm_eo)
            {
                printf("\n");
            }
            
            if (*s != '\0')
            {
                s += sizeof(dim_del) - 1;
            }

            e = strstr(s, dim_del);         
        }
    }
    else 
    {
        switch (statusCode)
        {
            case DIM_ST_PATH_NOT_FOUND:   print_scode(S20); break;
            case DIM_ST_NOT_DIR:          print_scode(S21); break;
            case DIM_ST_NOT_ENOUGH_RIGHT: print_scode(S22); break;
            case DIM_ST_UNKNOWN:          print_scode(S25); break;
            case DIM_ST_BAD_REQ:          print_scode(S30); break;
            default:                      print_scode(SXX); break;
        }
    }


    regfree(&re);

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


    // Main variables & buffers
    // -------------------------------------------------------------------------

    tMessage request;
    tMessage response;

    request.content = NULL;
    response.content = NULL;

    // -------------------------------------------------------------------------


    // Process request and results
    // -------------------------------------------------------------------------

    int result = create_request(&request, params.path);

    if (result != EOK)
    {
        print_ecode(result);
        return EXIT_FAILURE;
    }
    
    result = process_request(params.url, params.port, &request, &response);
    
    if (result != EOK)
    {
        free2(request.content);
        free2(response.content);
        print_ecode(result);
        return EXIT_FAILURE;
    }

    // -------------------------------------------------------------------------


    // Print results
    // -------------------------------------------------------------------------

    result = print_results(&response);

    if (result != EOK)
    {
        free2(request.content);
        free2(response.content);
        print_ecode(result);
        return EXIT_FAILURE;
    }

    // -------------------------------------------------------------------------


    // Free resources
    // -------------------------------------------------------------------------

    free2(request.content);
    free2(response.content);

    // -------------------------------------------------------------------------

    return EXIT_SUCCESS;
}

/* end of client.c */
