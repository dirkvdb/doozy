/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */

#ifndef DOOZY_MD5_H
#define DOOZY_MD5_H

#include <string>
#include <cinttypes>

class Md5
{
public:
    Md5();

    void update(const uint8_t *data, uint64_t size);
    void finalize(uint8_t result[16]);

    static std::string toString(const uint8_t data[16]);

private:
    const uint8_t* body(const uint8_t* data, uint64_t size);

    uint32_t    m_lo;
    uint32_t    m_hi;
    uint32_t    m_a;
    uint32_t    m_b;
    uint32_t    m_c;
    uint32_t    m_d;
    uint8_t     m_buffer[64];
#if !defined(__i386__) && !defined(__x86_64__) && !defined(__vax__)
    uint32_t    m_block[16];
#endif
};

#endif
