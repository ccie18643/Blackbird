/*
 * Copyright (c) 2001 - 2003
 *      Sebastian Majewski. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Sebastian Majewski
 *      and his contributors.
 * 4. Neither the name of Sebastian Majewski nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifndef _NETZ_SPX_SUPPORT_H_
#define _NETZ_SPX_SUPPORT_H_


#include "spx.h"


/*
 * SPX protocol supoort class
 */


class c_spx_header
{
protected:
    s_spx_header* header;

public:
    c_spx_header(byte*);
    c_spx_header(s_spx_header*);

    byte get_ccflags();
    byte get_dtype();
    word get_srcid();
    word get_dstid();
    word get_seq();
    word get_ack();
    word get_alloc();

    void set_ccflags(byte);
    void set_dtype(byte);
    void set_srcid(word);
    void set_dstid(word);
    void set_seq(word);
    void set_ack(word);
    void set_alloc(word);

    byte get_ccflag_eom();
    byte get_ccflag_ack();
    byte get_ccflag_sys();

    void set_ccflag_eom(byte);
    void set_ccflag_ack(byte);
    void set_ccflag_sys(byte);
};


#endif /* _NETZ_IPX_SUPPORT_H_ */

