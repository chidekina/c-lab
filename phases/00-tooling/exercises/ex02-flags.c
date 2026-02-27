#include <stdio.h>

/* Exercise: this file has intentional issues.
 * 1. Compile with: gcc -Wall -Wextra ex02-flags.c -o ex02
 * 2. Read each warning. Fix them one by one.
 * 3. Compile with: gcc -Wall -Wextra -fsanitize=address,undefined ex02-flags.c -o ex02 && ./ex02
 * 4. What does the sanitizer report?
 */

int add(int a, int b) {
    int result;          /* uninitialized — intentional */
    result = a + b;
    return result;
}

int main(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    printf("sum: %d\n", add(2, 3));
    printf("arr[5] = %d\n", arr[5]);  /* out-of-bounds — intentional */
    return 0;
}
