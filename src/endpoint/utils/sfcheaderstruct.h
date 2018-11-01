#ifndef IRONHIDE_SFCHEADERSTRUCT_H
#define IRONHIDE_SFCHEADERSTRUCT_H

#include <cstdint>

/**
 * Struct that represent a sfc header
 * reference: https://tools.ietf.org/id/draft-ietf-sfc-nsh-17.html
 */

struct sfc_header {
    /**************************BASE HEADER*************************/
    uint16_t ver:2,         // protocol version
            oam:1,         // OAM bit
            u1:1,          // unused
            ttl:6,         // number of sf traversals
            len:6;         // total length of SBH, SH, CH in 4-byte words
    uint8_t  u2:4,
            md_type:4;

    uint8_t  prot;          //protocol used to encaps data

    /*************************SFC HEADER*************************/
    uint32_t p_id:24,       //Service Path Identifier (SPI): 24 bits
            si:8;          //Service Index (SI): 8 bits

    /***********************CONTEXT HEADER***********************/
    uint32_t source_address,
            destination_address;
    uint16_t source_port,
            destination_port;

    uint8_t  direction_flag:1;

    uint32_t other:24;
}__attribute__((__packed__)); // disallow optimization of byte positioning

#endif //IRONHIDE_SFCHEADERSTRUCT_H
