//NO_TEAMBOOK
#include <bits/stdc++.h>
using namespace std;
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)
typedef long long i64;

typedef double ld;

struct base {
    ld re, im;
    base(){}
    base(ld re) : re(re), im(0) {}
    base(ld re, ld im) : re(re), im(im) {}

    base operator+(const base& o) const { return {re+o.re, im+o.im}; }
    base operator-(const base& o) const { return {re-o.re, im-o.im}; }
    base operator*(const base& o) const {
        return {
            re*o.re - im*o.im,
            re*o.im + im*o.re
        };
    }
};

//BEGIN_CODE
const int sz = 1<<20;

int revb[sz];
vector<base> ang[21];

void init(int n) {
    int lg = 0;
    while ((1<<lg) != n) {
        ++lg;
    }
    forn(i, n) {
        revb[i] = (revb[i>>1]>>1)^((i&1)<<(lg-1));
        cerr << revb[i] << ' ';
    }
    cerr << '\n';

    ld e = M_PI * 2 / n;
    ang[lg].resize(n);
    forn(i, n) {
        ang[lg][i] = { cos(e * i), sin(e * i) };
    }

    for (int k = lg - 1; k >= 0; --k) {
        ang[k].resize(1 << k);
        forn(i, 1<<k) {
            ang[k][i] = ang[k+1][i*2];
        }
    }
}

void fft_rec(base *a, int lg, bool rev) {
    if (lg == 0) {
        return;
    }
    int len = 1 << (lg - 1);
    fft_rec(a, lg-1, rev);
    fft_rec(a+len, lg-1, rev);

    forn(i, len) {
        base w = ang[lg][i];
        if (rev) w.im *= -1;
        base u = a[i];
        base v = a[i+len] * w;
        a[i] = u + v;
        a[i+len] = u - v;
    }
}

void fft(base *a, int n, bool rev) {
    forn(i, n) {
        int j = revb[i];
        if (i < j) swap(a[i], a[j]);
    }
    int lg = 0;
    while ((1<<lg) != n) {
        ++lg;
    }
    fft_rec(a, lg, rev);
    if (rev) forn(i, n) {
        a[i] = a[i] * (1.0 / n);
    }
}

const int maxn = 1050000;

int n;
base a[maxn];
base b[maxn];

void test() {
    int n = 8;
    init(n);
    base a[8] = {1,3,5,2,4,6,7,1};
    fft(a, n, 0);
    forn(i, n) cout << a[i].re << " "; cout << endl;
    forn(i, n) cout << a[i].im << " "; cout << endl;
    // 29 -5.82843 -7 -0.171573 5 -0.171573 -7 -5.82843
    // 0 -3.41421 6 0.585786 0 -0.585786 -6 3.41421
}
//END_CODE

int main() {
    test();
}
