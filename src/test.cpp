#include <stdio.h>

typedef struct _test {
        int x[3];

        int &operator[] (int idx) { return x[idx]; }
} test;

void foo(int *arr) {
        printf("kkk %lld %lld\n", arr[0], arr[1]);
}

int main(void) {
        test a {1, 2, 3};
        test b { 2 };
        printf("%d %d %d %d %d %d\n", a[0], a[1], a[2], b[0], b[1], b[2]);

        foo({1,2});
        return 0;
}

