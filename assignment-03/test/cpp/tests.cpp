// MOVABLE: not dead but dominates exits and uses
int test1(int n, bool cond) {
    int result = 0;
    int temp = 0;

    while (true) {
        temp = n * 2; // I to consider
        if (cond) break;
    }

    return temp; // used after loop
}

// MOVABLE: all 3 instructions
void test2(int n, bool cond) {
    int a, b;
    int temp = 0;

    while (true) {
        a = n * 2;
        b = a + 1;
        temp = b * 2; 

        if (cond) break;
    }
}


// NOT MOVABLE: dead after but does not dominate all uses (also not defined only once)
void test3(bool cond){
    int result = 0, x = 0, a = 10;

    for (int x = 0; x < 20; x++)  {
        int temp;
        result += temp; // I to consider

        if (cond) 
            temp = a * 2; // Defined only in one branch
    }
}

// NOT MOVABLE: multiple assignment (also not dominate exits and uses)
void test4(int n) {
    int x, y;

    for (int a = 0; a < 20; a++) {
        if (a == 4) 
            x = 3 + n; // I to consider
        else
            x = 4 + n; // I to consider

        y = x * 3;
    }
}

