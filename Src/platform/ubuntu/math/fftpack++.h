/* 
 * This file is based largely on the following software distribution:
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *                              FFTPACK
 * 
 * Reference                                                                                                                        
 *	P.N. Swarztrauber, Vectorizing the FFTs, in Parallel Computations
 *    (G. Rodrigue, ed.), Academic Press, 1982, pp. 51--83.                                                                                                                   
 * 
 *     http://www.netlib.org/fftpack/
 * 
 * Updated to single, double, and extended precision,
 * and translated to ISO-Standard C/C++ (without aliasing)
 * on 10 October 2005 by Andrew Fernandes <andrew_AT_fernandes.org>
 * 
 *                   Version 4  April 1985
 * 
 *      A Package of Fortran Subprograms for the Fast Fourier
 *       Transform of Periodic and other Symmetric Sequences
 * 
 *                          by
 * 
 *                   Paul N Swarztrauber
 * 
 *   National Center for Atmospheric Research, Boulder, Colorado 80307,
 * 
 *    which is sponsored by the National Science Foundation
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 * There appears to be no explicit license for FFTPACK. However, the
 * package has been incorporated verbatim into a large number of software
 * systems over the years with numerous types of license without complaint
 * from the original author; therefore it would appear
 * that the code is effectively public domain. If you are in doubt,
 * however, you will need to contact the author or the  National Center
 * for Atmospheric Research to be sure.
 * 
 * All the changes from the original FFTPACK to the current file
 * fall under the following BSD-style open-source license:
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 * Copyright (c) 2005, Andrew Fernandes (andrew@fernandes.org);
 * All rights reserved.
 *  
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * - Neither the name of the North Carolina State University nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
typedef float real_t;
#include <math.h>

static void s_radf2(int *ido, int *l1, real_t *cc, real_t *ch, real_t *wa1);
static void s_radf3(int *ido, int *l1, real_t *cc, real_t *ch, real_t *wa1, real_t *wa2);
static void s_radf4(int *ido, int *l1, real_t *cc, real_t *ch, real_t *wa1, real_t *wa2,
                    real_t *wa3);
static void s_radf5(int *ido, int *l1, real_t *cc, real_t *ch, real_t *wa1, real_t *wa2,
                    real_t *wa3, real_t *wa4);
static void s_radfg(int *ido, int *ip, int *l1, int *idl1, real_t *cc, real_t *c1, real_t *c2,
                    real_t *ch, real_t *ch2, real_t *wa);

static void s_radb2(int *ido, int *l1, real_t *cc, real_t *ch, real_t *wa1) {
    /* System generated locals */
    int cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    /* Local variables */
    int i__, k, ic;
    real_t ti2, tr2;
    int idp2;

    /* Parameter adjustments */
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = 1 + ch_dim1 * (1 + ch_dim2);
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = 1 + cc_dim1 * 3;
    cc -= cc_offset;
    --wa1;

    /* Function Body */
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1]
                                        + cc[*ido + ((k << 1) + 2) * cc_dim1];
    ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1]
                                                - cc[*ido + ((k << 1) + 2) * cc_dim1];
/* L101: */
    }
    if ((i__1 = *ido - 2) < 0) {
    goto L107;
    } else if (i__1 == 0) {
    goto L105;
    } else {
    goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        ic = idp2 - i__;
        ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + ((k << 1) + 1) * cc_dim1]
                                                + cc[ic - 1 + ((k << 1) + 2) * cc_dim1];
        tr2 = cc[i__ - 1 + ((k << 1) + 1) * cc_dim1] - cc[ic - 1 + ((k << 1) + 2) * cc_dim1];
        ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + ((k << 1) + 1) * cc_dim1]
                                                - cc[ic + ((k << 1) + 2) * cc_dim1];
        ti2 = cc[i__ + ((k << 1) + 1) * cc_dim1] + cc[ic + ((k << 1) + 2) * cc_dim1];
        ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * tr2 - wa1[i__ - 1] * ti2;
        ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * ti2 + wa1[i__ - 1] * tr2;
/* L103: */
    }
/* L104: */
    }
    if (*ido % 2 == 1) {
    return;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    ch[*ido + (k + ch_dim2) * ch_dim1] = cc[*ido + ((k << 1) + 1) * cc_dim1]
                                            + cc[*ido + ((k << 1) + 1) * cc_dim1];
    ch[*ido + (k + (ch_dim2 << 1)) * ch_dim1] = -(cc[((k << 1) + 2) * cc_dim1 + 1]
                                                    + cc[((k << 1) + 2) * cc_dim1 + 1]);
/* L106: */
    }
L107:
    return;
} /* radb2_ */

/* Subroutine */ static void s_radb3(int *ido, int *l1, real_t *cc,
                                        real_t *ch, real_t *wa1, real_t *wa2) {
    /* Initialized data */

    static real_t taur = -0.5f;
    static real_t taui = 0.8660254037844386467637231707529361834710262690519031402790348975f;

    /* System generated locals */
    int cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    /* Local variables */
    int i__, k, ic;
    real_t ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
    int idp2;

    /* Parameter adjustments */
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = 1 + ch_dim1 * (1 + ch_dim2);
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = 1 + (cc_dim1 << 2);
    cc -= cc_offset;
    --wa1;
    --wa2;

    /* Function Body */
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    tr2 = cc[*ido + (k * 3 + 2) * cc_dim1] + cc[*ido + (k * 3 + 2) * cc_dim1];
    cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
    ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
    ci3 = taui * (cc[(k * 3 + 3) * cc_dim1 + 1] + cc[(k * 3 + 3) * cc_dim1 + 1]);
    ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
    ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
/* L101: */
    }
    if (*ido == 1) {
    return;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        ic = idp2 - i__;
        tr2 = cc[i__ - 1 + (k * 3 + 3) * cc_dim1] + cc[ic - 1 + (k * 3 + 2) * cc_dim1];
        cr2 = cc[i__ - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
        ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 3 + 1) *cc_dim1] + tr2;
        ti2 = cc[i__ + (k * 3 + 3) * cc_dim1] - cc[ic + (k * 3 + 2) * cc_dim1];
        ci2 = cc[i__ + (k * 3 + 1) * cc_dim1] + taur * ti2;
        ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 3 + 1) * cc_dim1] + ti2;
        cr3 = taui * (cc[i__ - 1 + (k * 3 + 3) * cc_dim1] - cc[ic - 1 + (k * 3 + 2) * cc_dim1]);
        ci3 = taui * (cc[i__ + (k * 3 + 3) * cc_dim1] + cc[ic + (k * 3 + 2) * cc_dim1]);
        dr2 = cr2 - ci3;
        dr3 = cr2 + ci3;
        di2 = ci2 + cr3;
        di3 = ci2 - cr3;
        ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * dr2 - wa1[i__ - 1] * di2;
        ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * di2 + wa1[i__ - 1] * dr2;
        ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 2] * dr3 - wa2[i__ - 1] * di3;
        ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 2] * di3 + wa2[i__ - 1] * dr3;
/* L102: */
    }
/* L103: */
    }
    return;
} /* radb3_ */

/* Subroutine */ static void s_radb4(int *ido, int *l1, real_t *cc,
                                        real_t *ch, real_t *wa1, real_t *wa2, real_t *wa3) {
    /* Initialized data */

    static real_t sqrt2 = 1.41421356237309504880168872420969807856967187536948073176679738f;

    /* System generated locals */
    int cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    /* Local variables */
    int i__, k, ic;
    real_t ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    int idp2;

    /* Parameter adjustments */
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = 1 + ch_dim1 * (1 + ch_dim2);
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = 1 + cc_dim1 * 5;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    /* Function Body */
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[*ido + ((k << 2) + 4) * cc_dim1];
    tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[*ido + ((k << 2) + 4) * cc_dim1];
    tr3 = cc[*ido + ((k << 2) + 2) * cc_dim1] + cc[*ido + ((k << 2) + 2) *cc_dim1];
    tr4 = cc[((k << 2) + 3) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 + 1];
    ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
    ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 - tr4;
    ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
    ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 + tr4;
/* L101: */
    }
    if ((i__1 = *ido - 2) < 0) {
    goto L107;
    } else if (i__1 == 0) {
    goto L105;
    } else {
    goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        ic = idp2 - i__;
        ti1 = cc[i__ + ((k << 2) + 1) * cc_dim1] + cc[ic + ((k << 2) + 4) * cc_dim1];
        ti2 = cc[i__ + ((k << 2) + 1) * cc_dim1] - cc[ic + ((k << 2) + 4) * cc_dim1];
        ti3 = cc[i__ + ((k << 2) + 3) * cc_dim1] - cc[ic + ((k << 2) + 2) * cc_dim1];
        tr4 = cc[i__ + ((k << 2) + 3) * cc_dim1] + cc[ic + ((k << 2) + 2) * cc_dim1];
        tr1 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] - cc[ic - 1 + ((k << 2) + 4) * cc_dim1];
        tr2 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] + cc[ic - 1 + ((k << 2) + 4) * cc_dim1];
        ti4 = cc[i__ - 1 + ((k << 2) + 3) * cc_dim1] - cc[ic - 1 + ((k << 2) + 2) * cc_dim1];
        tr3 = cc[i__ - 1 + ((k << 2) + 3) * cc_dim1] + cc[ic - 1 + ((k << 2) + 2) * cc_dim1];
        ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
        cr3 = tr2 - tr3;
        ch[i__ + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
        ci3 = ti2 - ti3;
        cr2 = tr1 - tr4;
        cr4 = tr1 + tr4;
        ci2 = ti1 + ti4;
        ci4 = ti1 - ti4;
        ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * cr2 - wa1[i__ - 1] * ci2;
        ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * ci2 + wa1[i__ - 1] * cr2;
        ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 2] * cr3 - wa2[i__ - 1] * ci3;
        ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 2] * ci3 + wa2[i__ - 1] * cr3;
        ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 2] * cr4 - wa3[i__ - 1] * ci4;
        ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 2] * ci4 + wa3[i__ - 1] * cr4;
/* L103: */
    }
/* L104: */
    }
    if (*ido % 2 == 1) {
    return;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    ti1 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 + 1];
    ti2 = cc[((k << 2) + 4) * cc_dim1 + 1] - cc[((k << 2) + 2) * cc_dim1 + 1];
    tr1 = cc[*ido + ((k << 2) + 1) * cc_dim1] - cc[*ido + ((k << 2) + 3) *cc_dim1];
    tr2 = cc[*ido + ((k << 2) + 1) * cc_dim1] + cc[*ido + ((k << 2) + 3) *cc_dim1];
    ch[*ido + (k + ch_dim2) * ch_dim1] = tr2 + tr2;
    ch[*ido + (k + (ch_dim2 << 1)) * ch_dim1] = sqrt2 * (tr1 - ti1);
    ch[*ido + (k + ch_dim2 * 3) * ch_dim1] = ti2 + ti2;
    ch[*ido + (k + (ch_dim2 << 2)) * ch_dim1] = -sqrt2 * (tr1 + ti1);
/* L106: */
    }
L107:
    return;
} /* radb4_ */

/* Subroutine */ static void s_radb5(int *ido, int *l1, real_t *cc, real_t *ch, real_t *wa1,
                                        real_t *wa2, real_t *wa3, real_t *wa4) {
    /* Initialized data */

    static real_t tr11 = 0.3090169943749474241022934171828195886015458990288143106772431137f;
    static real_t ti11 = 0.9510565162951535721164393337938214340569863412575022244730564442f;
    static real_t tr12 = -0.8090169943749474241022934171828190588601545899028814310677431135f;
    static real_t ti12 = 0.5877852522924731291687059546390727685976524376431459107227248076f;

    /* System generated locals */
    int cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    /* Local variables */
    int i__, k, ic;
    real_t ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4,
            ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
    int idp2;

    /* Parameter adjustments */
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = 1 + ch_dim1 * (1 + ch_dim2);
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = 1 + cc_dim1 * 6;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    /* Function Body */
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    ti5 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 3) * cc_dim1 + 1];
    ti4 = cc[(k * 5 + 5) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
    tr2 = cc[*ido + (k * 5 + 2) * cc_dim1] + cc[*ido + (k * 5 + 2) * cc_dim1];
    tr3 = cc[*ido + (k * 5 + 4) * cc_dim1] + cc[*ido + (k * 5 + 4) * cc_dim1];
    ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 + tr3;
    cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
    cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
    ci5 = ti11 * ti5 + ti12 * ti4;
    ci4 = ti12 * ti5 - ti11 * ti4;
    ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
    ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
    ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
    ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
/* L101: */
    }
    if (*ido == 1) {
    return;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        ic = idp2 - i__;
        ti5 = cc[i__ + (k * 5 + 3) * cc_dim1] + cc[ic + (k * 5 + 2) * cc_dim1];
        ti2 = cc[i__ + (k * 5 + 3) * cc_dim1] - cc[ic + (k * 5 + 2) * cc_dim1];
        ti4 = cc[i__ + (k * 5 + 5) * cc_dim1] + cc[ic + (k * 5 + 4) * cc_dim1];
        ti3 = cc[i__ + (k * 5 + 5) * cc_dim1] - cc[ic + (k * 5 + 4) * cc_dim1];
        tr5 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] - cc[ic - 1 + (k * 5 + 2) * cc_dim1];
        tr2 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] + cc[ic - 1 + (k * 5 + 2) * cc_dim1];
        tr4 = cc[i__ - 1 + (k * 5 + 5) * cc_dim1] - cc[ic - 1 + (k * 5 + 4) * cc_dim1];
        tr3 = cc[i__ - 1 + (k * 5 + 5) * cc_dim1] + cc[ic - 1 + (k * 5 + 4) * cc_dim1];
        ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 5 + 1) *cc_dim1] + tr2 + tr3;
        ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 5 + 1) * cc_dim1] + ti2 + ti3;
        cr2 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * tr3;
        ci2 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
        cr3 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * tr3;
        ci3 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
        cr5 = ti11 * tr5 + ti12 * tr4;
        ci5 = ti11 * ti5 + ti12 * ti4;
        cr4 = ti12 * tr5 - ti11 * tr4;
        ci4 = ti12 * ti5 - ti11 * ti4;
        dr3 = cr3 - ci4;
        dr4 = cr3 + ci4;
        di3 = ci3 + cr4;
        di4 = ci3 - cr4;
        dr5 = cr2 + ci5;
        dr2 = cr2 - ci5;
        di5 = ci2 - cr5;
        di2 = ci2 + cr5;
        ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * dr2 - wa1[i__ - 1] * di2;
        ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 2] * di2 + wa1[i__ - 1] * dr2;
        ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 2] * dr3 - wa2[i__ - 1] * di3;
        ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 2] * di3 + wa2[i__ - 1] * dr3;
        ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 2] * dr4 - wa3[i__ - 1] * di4;
        ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 2] * di4 + wa3[i__ - 1] * dr4;
        ch[i__ - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 2] * dr5 - wa4[i__ - 1] * di5;
        ch[i__ + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 2] * di5 + wa4[i__ - 1] * dr5;
/* L102: */
    }
/* L103: */
    }
    return;
} /* radb5_ */

/* Subroutine */ static void s_radbg(int *ido, int *ip, int *l1, int * idl1, real_t *cc,
                                    real_t *c1, real_t *c2, real_t *ch,  real_t *ch2, real_t *wa) {
    /* Initialized data */

    static real_t tpi = 6.283185307179586476925286766559005768394338798750116419498891846;

    /* System generated locals */
    int ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1, c1_dim2, c1_offset,
            c2_dim1, c2_offset, ch2_dim1, ch2_offset, i__1, i__2, i__3;

    /* Local variables */
    int i__, j, k, l, j2, ic, jc, lc, ik, is;
    real_t dc2, ai1, ai2, ar1, ar2, ds2;
    int nbd;
    real_t dcp, arg, dsp, ar1h, ar2h;
    int idp2, ipp2, idij, ipph;

    /* Parameter adjustments */
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = 1 + ch_dim1 * (1 + ch_dim2);
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = 1 + c1_dim1 * (1 + c1_dim2);
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = 1 + cc_dim1 * (1 + cc_dim2);
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = 1 + ch2_dim1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = 1 + c2_dim1;
    c2 -= c2_offset;
    --wa;

    /* Function Body */
    arg = tpi / (real_t) (*ip);
    dcp = cos(arg);
    dsp = sin(arg);
    idp2 = *ido + 2;
    nbd = (*ido - 1) / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    if (*ido < *l1) {
    goto L103;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
    i__2 = *ido;
    for (i__ = 1; i__ <= i__2; ++i__) {
        ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) *  cc_dim1];
/* L101: */
    }
/* L102: */
    }
    goto L106;
L103:
    i__1 = *ido;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) *  cc_dim1];
/* L104: */
    }
/* L105: */
    }
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    jc = ipp2 - j;
    j2 = j + j;
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        ch[(k + j * ch_dim2) * ch_dim1 + 1] = cc[*ido + (j2 - 2 + k *
            cc_dim2) * cc_dim1] + cc[*ido + (j2 - 2 + k * cc_dim2) *
            cc_dim1];
        ch[(k + jc * ch_dim2) * ch_dim1 + 1] = cc[(j2 - 1 + k * cc_dim2) *
            cc_dim1 + 1] + cc[(j2 - 1 + k * cc_dim2) * cc_dim1 + 1];
/* L107: */
    }
/* L108: */
    }
    if (*ido == 1) {
    goto L116;
    }
    if (nbd < *l1) {
    goto L112;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    jc = ipp2 - j;
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        i__3 = *ido;
        for (i__ = 3; i__ <= i__3; i__ += 2) {
        ic = idp2 - i__;
        ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1] =
                                            cc[i__ - 1 + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                                            + cc[ic - 1 + ((j<< 1) - 2 + k * cc_dim2) * cc_dim1];
        ch[i__ - 1 + (k + jc * ch_dim2) * ch_dim1] =
                        cc[i__ - 1 + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                        - cc[ic - 1 + ((j << 1) - 2 + k * cc_dim2) * cc_dim1];
        ch[i__ + (k + j * ch_dim2) * ch_dim1] =
                        cc[i__ + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                        - cc[ic + ((j << 1) - 2 + k * cc_dim2) * cc_dim1];
        ch[i__ + (k + jc * ch_dim2) * ch_dim1] =
                        cc[i__ + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                        + cc[ic + ((j << 1) - 2 + k * cc_dim2) * cc_dim1];
/* L109: */
        }
/* L110: */
    }
/* L111: */
    }
    goto L116;
L112:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    jc = ipp2 - j;
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        ic = idp2 - i__;
        i__3 = *l1;
        for (k = 1; k <= i__3; ++k) {
        ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1] =
                                            cc[i__ - 1 + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                                            + cc[ic - 1 + ((j << 1) - 2 + k * cc_dim2) * cc_dim1];
        ch[i__ - 1 + (k + jc * ch_dim2) * ch_dim1] =
                                            cc[i__ - 1 + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                                            - cc[ic - 1 + ((j << 1) - 2 + k * cc_dim2) * cc_dim1];

        ch[i__ + (k + j * ch_dim2) * ch_dim1] =
                                            cc[i__ + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                                            - cc[ic + ((j << 1) - 2 + k * cc_dim2) * cc_dim1];
        ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + ((j << 1) - 1 + k * cc_dim2) * cc_dim1]
                                                + cc[ic + ((j << 1) - 2 +  k * cc_dim2) * cc_dim1];
/* L113: */
        }
/* L114: */
    }
/* L115: */
    }
L116:
    ar1 = 1.0f;
    ai1 = 0.0f;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
    lc = ipp2 - l;
    ar1h = dcp * ar1 - dsp * ai1;
    ai1 = dcp * ai1 + dsp * ar1;
    ar1 = ar1h;
    i__2 = *idl1;
    for (ik = 1; ik <= i__2; ++ik) {
        c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + ar1 * ch2[ik + (ch2_dim1 << 1)];
        c2[ik + lc * c2_dim1] = ai1 * ch2[ik + *ip * ch2_dim1];
/* L117: */
    }
    dc2 = ar1;
    ds2 = ai1;
    ar2 = ar1;
    ai2 = ai1;
    i__2 = ipph;
    for (j = 3; j <= i__2; ++j) {
        jc = ipp2 - j;
        ar2h = dc2 * ar2 - ds2 * ai2;
        ai2 = dc2 * ai2 + ds2 * ar2;
        ar2 = ar2h;
        i__3 = *idl1;
        for (ik = 1; ik <= i__3; ++ik) {
        c2[ik + l * c2_dim1] += ar2 * ch2[ik + j * ch2_dim1];
        c2[ik + lc * c2_dim1] += ai2 * ch2[ik + jc * ch2_dim1];
/* L118: */
        }
/* L119: */
    }
/* L120: */
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    i__2 = *idl1;
    for (ik = 1; ik <= i__2; ++ik) {
        ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
/* L121: */
    }
/* L122: */
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    jc = ipp2 - j;
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        ch[(k + j * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * c1_dim1 + 1]
                                             - c1[(k + jc * c1_dim2) * c1_dim1 + 1];
        ch[(k + jc * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * c1_dim1 + 1]
                                              + c1[(k + jc * c1_dim2) * c1_dim1 + 1];
/* L123: */
    }
/* L124: */
    }
    if (*ido == 1) {
    goto L132;
    }
    if (nbd < *l1) {
    goto L128;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    jc = ipp2 - j;
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        i__3 = *ido;
        for (i__ = 3; i__ <= i__3; i__ += 2) {
        ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1] =
                                                    c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1]
                                                    - c1[i__ + (k + jc * c1_dim2) * c1_dim1];
        ch[i__ - 1 + (k + jc * ch_dim2) * ch_dim1] =
                                                    c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1]
                                                    + c1[i__ + (k + jc * c1_dim2) * c1_dim1];
        ch[i__ + (k + j * ch_dim2) * ch_dim1] =
                                                c1[i__ + (k + j * c1_dim2) * c1_dim1]
                                                + c1[i__ - 1 + (k + jc * c1_dim2) * c1_dim1];
        ch[i__ + (k + jc * ch_dim2) * ch_dim1] = c1[i__ + (k + j * c1_dim2) * c1_dim1]
                                                - c1[i__ - 1 + (k + jc * c1_dim2) * c1_dim1];
/* L125: */
        }
/* L126: */
    }
/* L127: */
    }
    goto L132;
L128:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
    jc = ipp2 - j;
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        i__3 = *l1;
        for (k = 1; k <= i__3; ++k) {
        ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1] = c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1]
                                                    - c1[i__ + (k + jc * c1_dim2) * c1_dim1];
        ch[i__ - 1 + (k + jc * ch_dim2) * ch_dim1] = c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1]
                                                    + c1[i__ + (k + jc * c1_dim2) * c1_dim1];
        ch[i__ + (k + j * ch_dim2) * ch_dim1] = c1[i__ + (k + j * c1_dim2) * c1_dim1]
                                                + c1[i__ - 1 + (k + jc * c1_dim2) * c1_dim1];
        ch[i__ + (k + jc * ch_dim2) * ch_dim1] = c1[i__ + (k + j * c1_dim2) * c1_dim1]
                                                - c1[i__ - 1 + (k + jc * c1_dim2) * c1_dim1];
/* L129: */
        }
/* L130: */
    }
/* L131: */
    }
L132:
    if (*ido == 1) {
    return;
    }
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
    c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
/* L133: */
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * ch_dim1 + 1];
/* L134: */
    }
/* L135: */
    }
    if (nbd > *l1) {
    goto L139;
    }
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
    is += *ido;
    idij = is;
    i__2 = *ido;
    for (i__ = 3; i__ <= i__2; i__ += 2) {
        idij += 2;
        i__3 = *l1;
        for (k = 1; k <= i__3; ++k) {
        c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1]
                                                    * ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1]
                                                    - wa[idij]
                                                    *ch[i__ + (k + j * ch_dim2) * ch_dim1];
        c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1]
                                                * ch[i__  + (k + j * ch_dim2) * ch_dim1]
                                                + wa[idij]
                                                * ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1];
/* L136: */
        }
/* L137: */
    }
/* L138: */
    }
    goto L143;
L139:
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
    is += *ido;
    i__2 = *l1;
    for (k = 1; k <= i__2; ++k) {
        idij = is;
        i__3 = *ido;
        for (i__ = 3; i__ <= i__3; i__ += 2) {
        idij += 2;
        c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1]
                                                    * ch[ i__ - 1 + (k + j * ch_dim2) * ch_dim1]
                                                    - wa[idij]
                                                    * ch[i__ + (k + j * ch_dim2) * ch_dim1];
        c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1]
                                                * ch[i__ + (k + j * ch_dim2) * ch_dim1] + wa[idij]
                                                * ch[i__ - 1 + (k + j * ch_dim2) * ch_dim1];
/* L140: */
        }
/* L141: */
    }
/* L142: */
    }
L143:
    return;
} /* radbg_ */
