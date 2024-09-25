/* These definitions need to be changed depending on the floating-point precision */

#ifndef SRC_PLATFORM_UBUNTU_MATH_RFFT_HPP_
#define SRC_PLATFORM_UBUNTU_MATH_RFFT_HPP_

typedef fftw_real real_t;
#include <fftw3.h>
#include <rfftw.h>
#include <iostream>
#include <vector>
#include <complex>

/*
The function init_rfft creates a plan for the r2c transform from fftw3 library.
@param N: The size of the input array.
@return: The plan for the r2c transform.
*/
fftw_plan init_rfft(int N) {
    // int N_out = N / 2 + 1;  // Output size for r2c transform

    // Allocate input and output arrays
    // in = fftw_alloc_real(N);
    // out = fftw_alloc_complex(N_out);

    // Create plan
    return rfftw_create_plan(N, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
}

void rfft_one_cycle(fftw_plan plan, real_t* in, real_t* out, int N) {
    rfftw_one(plan, in, out);
    // std::vector<double> x(in, in + N);
    // std::copy(x.begin(), x.end(), in);
    // fftw_execute(plan);
    // std::vector<double> y(out, out + N);
    // std::copy(out, out + N, y.begin());
}

#endif  // SRC_PLATFORM_UBUNTU_MATH_RFFT_HPP_

// void rfftf(int *n, real_t *r__, real_t *wsave, int *ifac) {
//     /* Parameter adjustments */
//     --ifac;
//     --wsave;
//     --r__;

//     /* Function Body */
//     if (*n == 1) {
//     return;
//     }
//     s_rfftf1(n, &r__[1], &wsave[1], &wsave[*n + 1], &ifac[1]);
//     return;
// } /* rfftf_ */

// static void s_rfftf1(int *n, real_t *c__, real_t *ch, real_t *wa, int *ifac) {
//     /* System generated locals */
//     int i__1;

//     /* Local variables */
//     int i__, k1, l1, l2, na, kh, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

//     /* Parameter adjustments */
//     --ifac;
//     --wa;
//     --ch;
//     --c__;

//     /* Function Body */
//     nf = ifac[2];
//     na = 1;
//     l2 = *n;
//     iw = *n;
//     i__1 = nf;
//     for (k1 = 1; k1 <= i__1; ++k1) {
//     kh = nf - k1;
//     ip = ifac[kh + 3];
//     l1 = l2 / ip;
//     ido = *n / l2;
//     idl1 = ido * l1;
//     iw -= (ip - 1) * ido;
//     na = 1 - na;
//     if (ip != 4) {
//         goto L102;
//     }
//     ix2 = iw + ido;
//     ix3 = ix2 + ido;
//     if (na != 0) {
//         goto L101;
//     }
//     s_radf4(&ido, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
//     goto L110;
// L101:
//     s_radf4(&ido, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3]);
//     goto L110;
// L102:
//     if (ip != 2) {
//         goto L104;
//     }
//     if (na != 0) {
//         goto L103;
//     }
//     s_radf2(&ido, &l1, &c__[1], &ch[1], &wa[iw]);
//     goto L110;
// L103:
//     s_radf2(&ido, &l1, &ch[1], &c__[1], &wa[iw]);
//     goto L110;
// L104:
//     if (ip != 3) {
//         goto L106;
//     }
//     ix2 = iw + ido;
//     if (na != 0) {
//         goto L105;
//     }
//     s_radf3(&ido, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2]);
//     goto L110;
// L105:
//     s_radf3(&ido, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2]);
//     goto L110;
// L106:
//     if (ip != 5) {
//         goto L108;
//     }
//     ix2 = iw + ido;
//     ix3 = ix2 + ido;
//     ix4 = ix3 + ido;
//     if (na != 0) {
//         goto L107;
//     }
//     s_radf5(&ido, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]);
//     goto L110;
// L107:
//     s_radf5(&ido, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]);
//     goto L110;
// L108:
//     if (ido == 1) {
//         na = 1 - na;
//     }
//     if (na != 0) {
//         goto L109;
//     }
//     s_radfg(&ido, &ip, &l1, &idl1, &c__[1], &c__[1], &c__[1], &ch[1], &ch[1], &wa[iw]);
//     na = 1;
//     goto L110;
// L109:
//     s_radfg(&ido, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c__[1], &c__[1], &wa[iw]);
//     na = 0;
// L110:
//     l2 = l1;
// /* L111: */
//     }
//     if (na == 1) {
//     return;
//     }
//     i__1 = *n;
//     for (i__ = 1; i__ <= i__1; ++i__) {
//     c__[i__] = ch[i__];
// /* L112: */
//     }
//     return;
// } /* rfftf1_ */


// static void init_rfft(int *n, real_t *wa, int *ifac) {
//     /* Initialized data */
 
//     static int ntryh[4] = { 4, 2, 3, 5 };

//     /* System generated locals */
//     int i__1, i__2, i__3;

//     /* Local variables */
//     int i__, j, k1, l1, l2, ib;
//     real_t fi;
//     int ld, ii, nf, ip, nl, is, nq, nr;
//     real_t arg;
//     int ido, ipm;
//     real_t tpi;
//     int nfm1;
//     real_t argh;
//     int ntry = 0;
//     real_t argld;

//     /* Parameter adjustments */
//     --ifac;
//     --wa;

//     /* Function Body */
//     nl = *n;
//     nf = 0;
//     j = 0;
// L101:
//     ++j;
//     if (j - 4 <= 0) {
//     goto L102;
//     } else {
//     goto L103;
//     }
// L102:
//     ntry = ntryh[j - 1];
//     goto L104;
// L103:
//     ntry += 2;
// L104:
//     nq = nl / ntry;
//     nr = nl - ntry * nq;
//     if (nr != 0) {
//     goto L101;
//     } else {
//     goto L105;
//     }
// L105:
//     ++nf;
//     ifac[nf + 2] = ntry;
//     nl = nq;
//     if (ntry != 2) {
//     goto L107;
//     }
//     if (nf == 1) {
//     goto L107;
//     }
//     i__1 = nf;
//     for (i__ = 2; i__ <= i__1; ++i__) {
//     ib = nf - i__ + 2;
//     ifac[ib + 2] = ifac[ib + 1];
// /* L106: */
//     }
//     ifac[3] = 2;
// L107:
//     if (nl != 1) {
//     goto L104;
//     }
//     ifac[1] = *n;
//     ifac[2] = nf;
//     tpi = 6.283185307179586476925286766559005768394338798750211619498891846f;
//     argh = tpi / (real_t) (*n);
//     is = 0;
//     nfm1 = nf - 1;
//     l1 = 1;
//     if (nfm1 == 0) {
//     return;
//     }
//     i__1 = nfm1;
//     for (k1 = 1; k1 <= i__1; ++k1) {
//     ip = ifac[k1 + 2];
//     ld = 0;
//     l2 = l1 * ip;
//     ido = *n / l2;
//     ipm = ip - 1;
//     i__2 = ipm;
//     for (j = 1; j <= i__2; ++j) {
//         ld += l1;
//         i__ = is;
//         argld = (real_t) ld * argh;
//         fi = 0.0f;
//         i__3 = ido;
//         for (ii = 3; ii <= i__3; ii += 2) {
//         i__ += 2;
//         fi += 1.0f;
//         arg = fi * argld;
//         wa[i__ - 1] = cosf(arg);
//         wa[i__] = sinf(arg);
// /* L108: */
//         }
//         is += ido;
// /* L109: */
//     }
//     l1 = l2;
// /* L110: */
//     }
//     return;
// } /* s_rffti1 */
