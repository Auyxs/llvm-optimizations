int bothGuarded(int x, int y, int N) {
    int a=0, b=0;

    for (int i=1; i<N; i++)
        a += x;

    for (int i=1; i<N; i++)
        b += y;

    return a + b;
}

int BothNotGuarded() {
    int a=0, b=0, c=0;

    for (int i=1; i<10; i++){
        a += 1;
        b += 2;
        b += 3;
        c += 4;
    }

    for (int i=1; i<10; i++){
        a += 10;
        a += 20;
        b += 30;
    }

    return a + b + c;
}


int guardedWhile(int n, int a, int b) {
  int i = 0;
  if (n > 0) {
    do {
      a = a + 5;
      i += 1;
    } while (i < n);
  }
  i = 0;
  if (n > 0) {
    do {
      b = b + 6;
      i += 1;
    } while (i < n);
  }

  return b;
}

void noDep(int a[], int b[], int n) {
  int  t = 0;

    for (int i=0; i<n; ++i) {
        a[i+1] = t;
    }

    for (int i=0; i<n; ++i) {
        b[i] = 2;
    }
}

void Dep(int a[], int b[], int n) {
  int  t = 0;

    for (int i=0; i<n; ++i) {
        a[i] = t;
    }

    for (int i=0; i<n; ++i) {
        b[i] = a[i + 3] + 1;
    }
}