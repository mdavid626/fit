/*
 * File:     webinfo.c
 * Date:     2011/03/25
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Webovy klient pro zjisteni informaci o objektu
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

#define MAX_PARAM 4
#define MAX_REDIRECT 5
#define RESPONSE_BUFFER_SIZE 2048
#define REGEX_BUFFER_SIZE 2048
#define DEFAULT_PORT 80
#define REQUEST_LENGTH 2048
#define PATH_BUFFER_SIZE 2048

#define SLENGTH "Content-Length"
#define SSERVER "Server"
#define SMOD "Last-Modified"
#define STYP "Content-Type"
#define SLOCATION "Location"

/** Program error codes */
enum tecodes
{
  EOK = 0,        /**< Without error */
  EPARAM,         /**< Error in parameters */
 
  ESOCCREATE,     /**< Error creating socket */
  EHOSTNAMEERROR, /**< Error resolving url */
  ESOCCONNECT,    /**< Socet Error on connect	*/
  ESOCWRITE,      /**< Socket Error on write */
  ESOCREAD,       /**< Socket Error on read */
  ESOCCLOSE,      /**< Error on close socket */
  
  EMEM,           /**< Out of memory */
  ETOORED,        /**< Too many redirects */
  ERESPONSE,      /**< Error in response*/
  EURL,           /**< Error in url*/
  
  EREGEX,         /**< Regex error */

  EUNKNOWN,       /**< Unknown error */
};

/** State codes of program */
enum tstates
{
  CLENGTH,       /**<  Object length */
  CSERVER,       /**<  Server Iden.*/
  CMOD,          /**<  Last modification */
  CTYP,          /**<  Content type */
  CALL,          /**<  Every from above*/
  CEMPTY         /**<  Empty */
};

/** Error messages */
const char *ECODEMSG[] =
{
  /* EOK */
  "Everything OK.\n",
  /* EPARAM */
  "Errors in parameters!\n",
  
  /* Error creating socket */
  "Unable to create socket!\n",
  /* Error gethostbyname */
  "Unable to resolve URL!\n",
  /* Socet Error on connect	*/
  "Unable to connect!\n",
  /* Socket Error on write */
  "Socket: Unable to write!\n",
  /* Socket Error on read */  
  "Socket: Unable to read!\n",
  /* Error on close socket */  
  "Socket: Unable to close!\n",
  
  /* Out of memory */
  "Out of memory. Sorry!\n",
  /* Too many redirects */
  "Too many redirects! Max allowed: 5.\n",
  /* Errors in response */
  "Response from server: not expected format!\n",
  /* Wrong URL format */
  "URL is not valid!\n",
  
  /* Regex error */
  "Regex error!\n",
  
  /* EUNKNOWN */
  "Unknown error!\n"
};

/**
 * Structura containing parameters from command line
 */
typedef struct params
{
  int filter[MAX_PARAM];  /**< filter results */
  char *url;              /**< URL */
  int ecode;              /**< erro code (enum tecodes) */
} tParams;

/**
 * Structura used to store request and response sent/received from server
 */
typedef struct message
{
  char *content;         /**< Received/sent to server */
  int length;            /**< Length of message */
} tMessage;


/**
 * Prints error message to stderr
 * @param ecode error code
 */
void printECode(int ecode)
{
  if (ecode < EOK || ecode > EUNKNOWN)
  { 
    ecode = EUNKNOWN; 
  }

  fprintf(stderr, "%s", ECODEMSG[ecode]);
} 

/**
 * Frees a pointer if it not NULL
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
 * Rounds a number to power of 2
 * @param n number to round
 */
int round2(int n)
{
  int result = 1;
  
  while (result < n)
  {
    result = result << 1;
  }
  
  return result;
}

/**
 * Allocates memory on heap
 * Return a char* to allocated memory
 * @param count memory size, it is rounded to power of 2
 */
char* charAlloc(int count)
{
  return (char*)malloc(sizeof(char) * round2(count));
}

/**
 * Processes arguments of command line
 * @param argc Argument count
 * @param argv Pole string with arguments
 */
tParams getParams(int argc, char *argv[])
{
  tParams result =
  { // init structure
    .ecode = EOK
  };

  int ch;
  int index = 0;
  
  opterr = 0;
  while ((ch = getopt(argc, argv, "lsmt")) != -1)
  {
    switch (ch)
	{
	  case 'l': 
				result.filter[index] = CLENGTH;
				index += 1;
				break;
				
	  case 's': 
				result.filter[index] = CSERVER;
				index += 1;
				break;
				
      case 'm': 
				result.filter[index] = CMOD;
				index += 1;
				break;
				
      case 't':
				result.filter[index] = CTYP;
				index += 1;
				break;	

      default: 
	          result.ecode = EPARAM;
	          break;		
	}
  }
  
  if (index == 0)
  {
    result.filter[index] = CALL;
	index += 1;
  }
  
  result.filter[index] = CEMPTY;
 
  int last = argc - 1;
  
  if (last == optind)
  {
	result.url = argv[last];
  }
  else
  {
    result.ecode = EPARAM;
  }

  return result;
}

/**
 * Process a regex
 * @param *string
 * @param *pattern
 * @param *buf
 */
int processRegex(char *string, char *pattern, char *buf)
{
  for (int i = 0; i < REGEX_BUFFER_SIZE; i++)
  {
    buf[i] = '\0';
  }
  
  int nmatch = 1;
  int match = 1;
  regmatch_t pmatch[nmatch + 1];
  regex_t re;
  
  if (regcomp(&re, pattern, REG_EXTENDED | REG_NEWLINE | REG_ICASE) != 0) 
  {
    return EUNKNOWN;
  }
  
  int status = regexec(&re, string, nmatch + 1, pmatch, 0);
  
  if (status == 0)
  {
    strncpy(buf, string + pmatch[match].rm_so, pmatch[match].rm_eo - pmatch[match].rm_so);
  }
  
  regfree(&re);
  
  return EOK;
}

/**
 * Process an URL, create the request
 * @param *url
 * @param **hostName
 * @param *port
 * @param *request
 */
int processURL(char *url, char **hostName, int *port, tMessage *request)
{
  int urlLength = strlen(url) + 1;
  *hostName = charAlloc(urlLength);
  
  if (*hostName == NULL)
  {
    return EMEM;
  }
  
  char *path = charAlloc(urlLength);
  
  if (path == NULL)
  {
    return EMEM;
  }
  
  char portSt[10] = "";

  // valid url?  
  regmatch_t pmatch[6];
  regex_t re;
  
  //char pattern[] = "^(http:\\/\\/)?([\\da-z\\.-]+\\.[a-z\\.]{2,6})(([\\/\\w \\?=.-]*)*\\/?)$";  
  char pattern[] = "^(http:\\/\\/)?([^:\\/\?]{3,})(:([0-9]{1,5}))?(\\/[^\\#]*)?([#\\].*)?$";
  
  if (regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) 
  {
    return EREGEX;
  }
  
  (*hostName)[0] = '\0';
  portSt[0] = '\0';
  path[0] = '\0';
  
  if (regexec(&re, url, 6, pmatch, 0) == 0)
  {
    // hostname
	int n = pmatch[2].rm_eo - pmatch[2].rm_so;
	(*hostName)[n] = '\0';
    strncpy(*hostName, url + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
	
	// port
	n = pmatch[4].rm_eo - pmatch[4].rm_so;
	strncpy(portSt, url + pmatch[4].rm_so, n);
	
	// path
	n = pmatch[5].rm_eo - pmatch[5].rm_so;
	path[n] = '\0';
	strncpy(path, url + pmatch[5].rm_so, n);
  } 
  
  regfree(&re);

  if (**hostName == '\0')
  {
    free2(path);
	return EURL;
  }
  
  if (*portSt != '\0')
  {
	*port = atoi(portSt);
	
	if (*port == 0)
	{
	  *port = DEFAULT_PORT;
	}
  }
  else
  {
    *port = DEFAULT_PORT;
  }
  
  if (path[0] != '\0')
  {
    // space to %20, semicolon to %3B
	char *newPath = charAlloc(PATH_BUFFER_SIZE);
	if (newPath == NULL)
	{
	  free2(*hostName);
	  free2(path);
	  return EMEM;
	}
	
	int i = 0;
	int j = 0;
	while (path[i] != '\0')
	{
	  if (path[i] == ' ')
	  {
	    // space
		newPath[j++] = '%';
		newPath[j++] = '2';
		newPath[j++] = '0';
	  }
	  else if (path[i] == ';')
	  {
	    // semicolon
		newPath[j++] = '%';
		newPath[j++] = '3';
		newPath[j++] = 'B';
	  }
	  else 
	  {
	    newPath[j] = path[i];
		j += 1;
	  }
	  i += 1;
	}
	newPath[j] = '\0';
	
	free2(path);
	path = newPath;
  }
  else
  {
	strcpy(path, "/");
  }
  
  request->content = charAlloc(REQUEST_LENGTH);	
  if (request->content == NULL)
  {
    free2(*hostName);
	free2(path);
    return EMEM;
  }
  
  // create request
  sprintf(request->content, "HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, *hostName);
  request->length = strlen(request->content);
  
  free2(path);
  
  return EOK;
}
 
/**
 * Process request, open socket, send data, receive data
 * @param *hostName
 * @param port
 * @param *request
 * @param *response
 */
int processRequest(char *hostName, int port, tMessage *request, tMessage *response)
{
  int s = 0;
  
  struct sockaddr_in sin; 
  struct hostent *hptr;
  
  // create socket
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
  {     
    return ESOCCREATE;
  }
  
  // set protocol family to Internet
  sin.sin_family = PF_INET;
  
  // set port number
  sin.sin_port = htons(port);  

  if ((hptr = gethostbyname(hostName)) == NULL)
  {
    return EHOSTNAMEERROR;
  }
  
  memcpy(&(sin.sin_addr), hptr->h_addr, hptr->h_length);
  
  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    close(s);
	return ESOCCONNECT;
  }
  
  // send message to server
  if (write(s, request->content, request->length) < 0) 
  {    
    close(s);
	return ESOCWRITE;
  }
  
  // read message from server
  if ((response->length = read(s, response->content, response->length)) < 0)
  {
    close(s);
    return ESOCREAD;
  }
  
  // close connection, clean up socket
  if (close(s) < 0) 
  { 
    return ESOCCLOSE;
  }
  
  return EOK;
}

/**
 * Print results to stdout
 * @param *response
 * @param *filter
 */
int printResults(tMessage *response, int *filter)
{ 
  // normalize respond
  // buffer for normalization
  char *norm = charAlloc(response->length + 1);
  
  if (norm == NULL)
  {
    return EMEM;
  }
  
  // all \r\n => \n
  int j = 0;
  for (int i = 0; i < response->length; i++)
  {
    if (response->content[i] != '\r')
	{
	  norm[j] = response->content[i];
	  j += 1;
	}
  }
  
  // at the end clear all \n
  if (j >= 2 && norm[j - 2] == '\n')
  {
    j -= 2;
  }
  
  norm[j] = '\0';
  
  free2(response->content);
  response->content = norm;
  response->length = j + 1;
  
  // buffer for regex
  char *regexBuf = charAlloc(REGEX_BUFFER_SIZE);
  
  if (regexBuf == NULL)
  {
    return EMEM;
  }
  
  // valid HTTP/1.1?
  processRegex(response->content, "^(HTTP/1.[01x] .*)$", regexBuf);
  
  if (*regexBuf == '\0')
  {
    // not valid HTTP/1.1
    return ERESPONSE;
  }

  // HTTP/1.x 4xx or 5xx?
  processRegex(response->content, "^HTTP/1.[01x] ([45][0-9]{2} .*)$", regexBuf);

  if (*regexBuf != '\0')
  {
    // code 4xx or 5xx
	fprintf(stderr, "Error: %s\n", regexBuf);
  }
  
  // code 2xx?
  /*processRegex(response->content, "^(HTTP/1.[01x] 2[0-9]{2} .*)$", regexBuf);
  
  if (*regexBuf == '\0')
  {
    // code not 2xx
    return ERESPONSE;
  }*/
  
  // everything is OK, print out
  int i = 0;
  bool found;
  while (filter[i] != CEMPTY && i < MAX_PARAM)
  {	
    found = false;
    switch (filter[i])
	{
	  case CLENGTH: 
					processRegex(response->content, "^("SLENGTH":.*)$", regexBuf);
					
					if (*regexBuf != '\0')
					{ // found
					  printf("%s", regexBuf);
					}
					else
					{
					  printf("%s: N/A", SLENGTH);
					}
					
	                break;
					
	  case CSERVER: 
	                processRegex(response->content,  "^("SSERVER":.*)$", regexBuf);
					
					if (*regexBuf != '\0')
					{ // found
					  printf("%s", regexBuf);
					}
					else
					{
					  printf("%s: N/A", SSERVER);
					}
					
	                break;
					
	  case CMOD: 
	                processRegex(response->content,  "^("SMOD":.*)$", regexBuf);
					
					if (*regexBuf != '\0')
					{ // found
					  printf("%s", regexBuf);
					}
					else
					{
					  printf("%s: N/A", SMOD);
					}
					
					break;
	  case CTYP: 
	                processRegex(response->content,  "^("STYP":.*)$", regexBuf);
					
					if (*regexBuf != '\0')
					{ // found
					  printf("%s", regexBuf);
					}
					else
					{
					  printf("%s: N/A", STYP);
					}
					
					break;					
	  case CALL: 
	                if (*regexBuf == '\0')
					{ // nothing special, code in response was 2xx
					  printf("%s", response->content);
					}
					else
					{ // code was 4xx or 5xx
					   // little hacking... skip the first line,
					   // because we have written it to stderr
					   /*
					   char *newContent = response->content;
					   
					   // find new line
					   while (*newContent != '\n' && *newContent != '\0')
					   {
					     newContent += 1;
					   }
					   
					   // skip new line
					   if (*newContent == '\n')
					   {
					     newContent += 1;
					   }
					   
					   printf("%s", newContent);*/
					   printf("%s", response->content);
					}
	                
	                break;	
	}
	
	printf("\n");
	i += 1;
  }
  
  return EOK;
}

/**
 * Based on response determine if should redirect
 * If yes, then *newURL is not NULL, it contains the new URL
 * @param *response
 * @param **newURL
 */
int getRedirect(tMessage *response, char **newURL)
{  
  // buffer for regex
  char *regexBuf = charAlloc(REGEX_BUFFER_SIZE);
  
  if (regexBuf == NULL)
  {
    return EMEM;
  }
  
  for (int i = 0; i < REGEX_BUFFER_SIZE; i++)
  {
    regexBuf[i] = '\0';
  }
  
  // no redirect
  *newURL = NULL;

  // HTTP/1.x 3xx?
  processRegex(response->content, "^(HTTP/1.[01x] 3[012]{2} .*)$", regexBuf);

  if (*regexBuf != '\0')
  {
    // code 3xx	
	// Looking for ==> Location: url
	processRegex(response->content, "^"SLOCATION": ([^\r]*)[\r]?$", regexBuf);
	
	if (*regexBuf != '\0')
	{
	  // 3xx and Location found
	  *newURL = charAlloc(strlen(regexBuf) + 1);
	  
	  if (*newURL == NULL)
	  {
	    return EMEM;
	  }
	  
	  strcpy(*newURL, regexBuf);
	}
  }
  
  return EOK;
}

/////////////////////////////////////////////////////////////////
/**
 * Main program
 */
int main(int argc, char *argv[])
{
  // Process parameters
  // --------------------------------------------------------------------------
  
  tParams params = getParams(argc, argv);
  
  if (params.ecode != EOK)
  { // wrong parameters
    printECode(params.ecode);
    return EXIT_FAILURE;
  }

  // -------------------------------------------------------------------------
  
  
  // Main variables & buffers
  // -------------------------------------------------------------------------

  char *hostName;
  int port;
  tMessage request;
  tMessage response;
 
  char *url = charAlloc(strlen(params.url) + 1);
  
  if (url == NULL)
  {
    printECode(EMEM);
    return EXIT_FAILURE;
  }
  
  strcpy(url, params.url);
  
  char *regexBuf = charAlloc(REGEX_BUFFER_SIZE);
  
  if (regexBuf == NULL)
  {
    free2(url);
    printECode(EMEM);
	return EXIT_FAILURE;
  }

  // -------------------------------------------------------------------------
  
  
  // Process request and results
  // -------------------------------------------------------------------------
  
  bool direct = true;
  int redirectCount = 0;
  int result;
  
  while (direct)
  {
    // only once (without redirect)
    direct = false;
  
    // process URL
	result = processURL(url, &hostName, &port, &request);
	
	if (result != EOK)
	{
	  free2(url);
	  printECode(result);
	  return EXIT_FAILURE;
	}
	
	// launch request
	response.content = charAlloc(RESPONSE_BUFFER_SIZE);;
	response.length = RESPONSE_BUFFER_SIZE;
	
	if (response.content == NULL)
	{
	  free2(url);
	  free2(hostName);
	  free2(request.content);
	  printECode(EMEM);
	  return EXIT_FAILURE;
	}
	
    result = processRequest(hostName, port, &request, &response);
	
	if (result == EOK)
	{
	  // redirect?
	  char *newURL = NULL;
	  result = getRedirect(&response, &newURL);
	  
	  if (result != EOK)
	  {
		free2(url);
	    free2(hostName);
	    free2(request.content);
	    free2(response.content);
		printECode(result);
		return EXIT_FAILURE;
	  }

	  if (newURL != NULL)
	  {
	    // yes, redirect
		free2(url);
	    free2(hostName);
	    free2(request.content);
	    free2(response.content);
		
		if (redirectCount >= MAX_REDIRECT)
		{		
		  // too many redirects
		  free2(newURL);
		  printECode(ETOORED);
		  return EXIT_FAILURE;
		}
		
		url = newURL;
		
	    direct = true;
		redirectCount += 1;
		continue;
	  }
	}
    else
	{
	  // no response from server
	  free2(url);
	  free2(hostName);
	  free2(request.content);
	  free2(response.content);	  
	  printECode(result);
      return EXIT_FAILURE;
	}
  }
  
  // -------------------------------------------------------------------------
  
  // Print results
  // -------------------------------------------------------------------------
   
  result = printResults(&response, params.filter); 
  
  if (result != EOK)
  {
    free2(url);
    free2(hostName);
    free2(request.content);
    free2(response.content);
    printECode(result);
	return EXIT_FAILURE;
  }
  
  // -------------------------------------------------------------------------
  
  
  // Free resources
  // -------------------------------------------------------------------------
  
  free2(url);
  free2(hostName);
  free2(request.content);
  free2(response.content);
  
  // -------------------------------------------------------------------------

  return EXIT_SUCCESS;
}

/* end of webinfo.c */
