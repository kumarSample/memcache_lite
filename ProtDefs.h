/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ProtDefs.h
 * Author: kumars
 *
 * Created on June 1, 2016, 1:52 PM
 */

#ifndef PROTDEFS_H
#define PROTDEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum {
        REQUEST = 0x80,
        RESPONSE = 0x81
    } binary_magic;

    
    typedef enum {
        RESPONSE_SUCCESS = 0x00,      // No error
        RESPONSE_KEY_ENOENT = 0x01,   // Key not found
        RESPONSE_KEY_EEXISTS = 0x02,  // Key exists
        RESPONSE_E2BIG = 0x03,        //Value too large
        RESPONSE_EINVAL = 0x04,       // Invalid arguments
        RESPONSE_NOT_STORED = 0x05,   //  Item not stored
        RESPONSE_DELTA_BADVAL = 0x06  // Incr/Decr on non-numeric value.
    } response_status;

    /**
     * Defintion of the different command opcodes.
     * See section 3.3 Command Opcodes
     */
    typedef enum {
        CMD_GET = 0x00,
        CMD_SET = 0x01,
        CMD_ADD = 0x02,
        CMD_REPLACE = 0x03,
        CMD_DELETE = 0x04,
        CMD_INCREMENT = 0x05,
        CMD_DECREMENT = 0x06,
        CMD_QUIT = 0x07,
        CMD_FLUSH = 0x08,
        CMD_GETQ = 0x09,
        CMD_NOOP = 0x0a,
        CMD_VERSION = 0x0b,
        CMD_GETK = 0x0c,
        CMD_GETKQ = 0x0d,

    } command;

    /**
     * Definition of the data types in the packet
     * See section 3.4 Data Types
     */
    typedef enum {
        RAW_BYTES = 0x00
    } datatypes;

 
    
    typedef union {
        struct {
            uint8_t magic;
            uint8_t opcode;
            uint16_t keylen;
            uint8_t extlen;
            uint8_t datatype;
            uint16_t reserved;
            uint32_t bodylen;
            uint32_t opaque;
            uint64_t cas;
        } request;
        uint8_t bytes[24];
    } request_header;

    /**
     * Definition of the header structure for a response packet.
     * See section 2
     */
    typedef union {
        struct {
            uint8_t magic;
            uint8_t opcode;
            uint16_t keylen;
            uint8_t extlen;
            uint8_t datatype;
            uint16_t status;
            uint32_t bodylen;
            uint32_t opaque;
            uint64_t cas;
        } response;
        uint8_t bytes[24];
    } response_header;

  
    
    typedef union {
        struct {
            request_header header;
        } message;
        uint8_t bytes[sizeof(request_header)];
    } request_no_extras;


    
    typedef union {
        struct {
            response_header header;
        } message;
        uint8_t bytes[sizeof(response_header)];
    } response_no_extras;


    typedef request_no_extras request_get;


    typedef union {
        struct {
            response_header header;
            struct {
                uint32_t flags;
            } body;
        } message;
        uint8_t bytes[sizeof(response_header) + 4];
    } response_get;

    typedef response_get response_getq;
    typedef response_get response_getk;
    typedef response_get response_getkq;
    
    typedef union {
        struct {
            request_header header;
            struct {
                uint32_t flags;
                uint32_t expiration;
            } body;
        } message;
        uint8_t bytes[sizeof(request_header) + 8];
    } request_set;

    typedef response_no_extras response_set;
    

#ifdef __cplusplus
}
#endif




#endif /* PROTDEFS_H */

