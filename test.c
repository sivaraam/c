#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

int *return_ref() {
    static int val = 10;
    printf("print in function: %d\n", val);
    return &val;
}

int main(void) {
    // 0. pointer conversion
    int a=320;
    char *p;
    p = (char*) &a;  // warning about 'incompatible pinters' without the cast
    printf("0. pointer conversion\n");
    printf("%d %d\n", *p, a); // takes the lower order byte of binary representation of 320 (1 0100 0000)
    printf("\n");
    // int width=2
    //printf("%2$*1$d\n", width, a);

    // 1. Anonymous pointer addition
    char *str = 3 + "Hello"; // str take the value of 'lo' (acts like array access)
    printf("1. Anonymous pointer addition\n");
    printf("%s\n", str);
    printf("\n");

    // 2. sequence point test
    int seq=10;
    printf("2. sequence point test\n");
    seq++, seq--, ++seq, --seq, printf("%d\n", seq);
    seq++ && seq-- && ++seq && printf("%d\n", seq);
    printf("\n");

    // 3. pointer difference & addition
    printf("3. pointer difference (addition)\n");
    printf("%td %td %ld\n", p, str, p - str); // pointer subtraction is legal . POINTER ADDITION IS ILLEGAL
    printf("\n");

    // 4. Parser issue
    printf("4. Parser issue\n");
    printf("%x\n", 0xe + 12);
    printf("\n");

    // 5. null termination test
    char arr[4];
    arr[0] = 'g';
    arr[1] = 'i';
    arr[2] = 'l';
    printf("5. null termination test\n");
    printf("%s\n", arr); // garbage after printing 'gil' in gcc. no garbage after 'gil' in clang
    printf("\n");

    // 6. unsigned test
    unsigned short i = -10; // assigning negative value is not an error
    // %hd prints -10
    // %u prints a value that interprets the 2's complement representation of -10 as it is (no sign bit)
    // %d ditto %u
    printf("6. unsigned test\n");
    printf("%hd %d %u\n", i, i, i);

    i++;
    printf("%hd %u\n", i, i);

    for(; i<-2; i--) // unsigned value is never less than -2
      printf("%d ", i);
    printf("\n");

    // 7. sequencing operator test
    printf("7. sequencing operator test\n");
    int ter_1 = 10, ter_2 = 20;
    ter_2 = ter_2++;
    ter_1++, printf("%d %d\n", ter_1, ter_2);
    printf("\n");

    // 8. Divide by zero undefined
    printf("8. Divide by zero undefined (?)\n");
//    printf("%d\n", 10/0);i
    printf("\n");

    // 9. Octal test
    printf("9. Octal test\n");
    printf("%o\n", 010);
    printf("\n");

    // 10. INT_MIN
    printf("9.INT_MIN\n");
    printf("%d\n", INT_MIN);
    printf("\n");

    // 11. Hexadecimal input
    int hex;
    printf("11. Hexadecimal Input\n");
    printf("Enter a hexadecimal number: ");
    scanf("%x", &hex);
    printf("%x\n", hex%16);
    printf("\n");

    // 12. Array test
    printf("12. Array test\n");
    int array[2] = {11, 12, 14, }; // seems to ignore the value 14
    printf("%d\n", arr[3]);
    printf("\n");

    // 13. sizeof string
    printf("13. sizeof string\n");
    printf("sizeof(\"hello\"): %zu\n", sizeof("hello"));
    printf("\n");

    // 14. switch block test
    printf("14. switch block test\n");
    int ch = 1;
    switch(ch) {
        auto int switch_block_var;
        case 1: printf("i = %d\n", switch_block_var);
                printf("\n");
                break;
    }

    // 15. struct without initialization
    printf("15. struct without initialization\n");
    struct something {
        char ch_val;
        int int_val;
    };
    struct something var;
    var.ch_val = 'a';
    printf("%c %zu\n", var.ch_val, sizeof(var));
    printf("\n");

    // 16. return local reference of function
    printf("16. return local reference of function\n");
    int *ptr = return_ref();
    (*ptr)++;
    return_ref();
    (*ptr)++;
    printf("main val: %d\n", *ptr);
    printf("\n");

    // 17. union in struct
    printf("17. union in struct\n");
    struct u_struct {
        int int_val;
        char ch_val;
        union {
            int var;
            char ch;
        } val;
    };
    printf("sizeof a struct with union: %zu\n", sizeof(struct u_struct));
    printf("\n");

    // 18. dereference a muli-dimensional array
    printf("18. dereference a muli-dimensional array\n");
    int md_array[2][2] = { 1, 2, 3, 4 };
    printf("%p %p %x\n", md_array, *md_array, **md_array);
    printf("\n");

    // 19. int malloc test
    printf("19. int malloc test\n");
    int *ptr_malloc = (int*) malloc(sizeof(int) * 10);
    *(ptr_malloc++) = 100;
    printf("*ptr = %d\n", *(ptr_malloc-1));
    //    free(ptr_malloc);      //*** Error in `./a.out': free(): invalid pointer: 0x00000000010ef834 ***
}
