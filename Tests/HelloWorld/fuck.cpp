int fuck(int x) {
  int a[10]={1,1};
  for (int i = 2; i < 10; i ++) {
    a[i] = a[i - 1] + a[i - 2];
    if (i % 2 == 0) {
      a[i] ++;
    } else {
      a[i] --;
    }
  }
  return a[x];
}
int f(int i, int j) {
    int y = 1;
    while (i--) {
        y = y * j;
    }
    return y;
}
