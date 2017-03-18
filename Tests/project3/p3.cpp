
int* foo(int b) {
	int a, *c = &b;
	for (int i = 0; i < 2; i ++) {
		if (i % 2) {
			c = &a;
			return 0;
		} else {
			c = &b;
		}
	}
	return &a;
}
int bar(int kkk) {
	int a[5] = {1,2,kkk,4,5}, b[5] = {-1,-2,-3,-4,-5};
	int c = a[1] ? a[1] : b[1], *d = b[1] ? a : b;

	int e = *d;
	*d = c;
	int *f = &e;

	for (int i = 0; i < 5; i ++) {
		if (a[i] % 2) {
			a[1] ++;
			c = a[i];
			d = &c;
			e = *d;
		} else {
			b[1] ++;
			c = b[i];
			d = a + i;
			e = *f;
			if (a[1] == 11) {
				return 5;
			}
		}
		f = &a[i];
	}

	switch(a[1]) {
		case 0: c ++;
		case 1: d = &c;break;
		case 2: c = a[1]; d = &e; break;
		default: break;
	}
	c = *d;
	e = *f;
	int fib[100] = {1,1};
	for (int i = 2; i < 100; i ++) {
		fib[i] = fib[i - 1] + fib[i - 2];
		if (fib[i] % 2) {
			a[1] ++;
			return a[2];
		}
	}
	return 3;
}
