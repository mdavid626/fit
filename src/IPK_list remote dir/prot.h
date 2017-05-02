/*
 * File:     prot.h
 * Date:     2011-04-15
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Print content of remote directory - protocol
 */

#ifndef PROT_H
#define PROT_H

const char dim_version[] = "DIM/1.0";
const char dim_request_kw[] = "CONTENT";
const char dim_request_par[] = "Path:";
const char dim_del[] = "\r\n";
const char dim_end[] = "\r\n\r\n";

const char dim_request_format[] = "%s %s%s%s %s%s"; // dim_request_kw, dim_version, dim_del, dim_request_par, path, dim_end
const char dim_response_header_format[] = "%s %d%s"; // dim_version, status code, dim_del

const char dim_request_pattern[] = "^%s %s%s%s (.*)%s$"; // dim_request_kw, dim_version, dim_del, dim_request_par, dim_end
const char dim_response_pattern[] = "^%s ([0-9]{1,2})%s(.*)%s$"; // dim_version, dim_del, dim_end

#define DIM_ST_OK               10
#define DIM_ST_PATH_NOT_FOUND   20
#define DIM_ST_NOT_DIR          21
#define DIM_ST_NOT_ENOUGH_RIGHT 22
#define DIM_ST_UNKNOWN          25
#define DIM_ST_BAD_REQ          30

#endif // PROT_H
