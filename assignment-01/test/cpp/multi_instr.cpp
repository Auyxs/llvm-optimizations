void test_multiInstr(int x){
    int a1 = x + 1;
    int b1 = a1 - 1;
    int a2 = 1 + x;
    int b2 = a2 - 1;
    int c1 = b2 * b1;

    int a3 = x - 1;
    int b3 = a3 + 1;
    int b4 = 1 + a3;
    int c2 = b4 * b3;

    int a5 = 1 - x;
    int b5 = 1 - a5;
    int c5 = b5 * b4;
}

void test_noMultiInstr(int x){
    int a1 = x + 1;
    int a2 = 1 + x;
    int b1 = a1 + 1;
    int b2 = 1 - a2;
    int c1 = b2 * b1;

    int a3 = x - 1;
    int b3 = a3 - 1;
    int a4 = 1 - x;
    int b4 = 1 + a4;
    int c3 = b3 * b4;
}