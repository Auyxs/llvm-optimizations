// MOVABLE: not dead but dominates exits and uses
int test1(int n) {
    int result = 0;
    int temp = 0;

    for (int x = 0; x < 20; x++)  {
        temp = n * 2; // I to consider
    }

    return temp; // used after loop
}

// MOVABLE: all 3 instructions
int test2(int n) {
    int a, b;
    int temp = 0;

    for (int x = 0; x < 20; x++)  {
        a = n * 2;
        b = a + 1;
        temp = b * 2; 
    }
    return 0;
}


// NOT MOVABLE: dead after but does not dominate all uses (also not defined only once)
void test3(bool cond){
    int result = 0, x = 0, a = 10;
    int temp;

    for (int x = 0; x < 20; x++)  {
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

