#include <bits/stdc++.h>

using namespace std;

int n;
int a[1000];

int main() {
    n = 999;
    cout << n << endl;
    for (int i = 0; i < n; ++i) a[i] = rand() % 10 + 1;
    for (int i = 0; i < n; ++i) cout << a[i] << ' ';
    cout << endl;

    int s = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            s += __gcd(a[i], a[j]);
        }
    }
    cerr << s << endl;
}