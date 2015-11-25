clear && gcc -Wall -pedantic -Wextra -Werror test.c && ./a.out > results.txt && diff -u results_orig.txt results.txt
