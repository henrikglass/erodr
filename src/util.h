#ifndef UTIL_H
#define UTIL_H

int min(int a, int b);
int max(int a, int b);

/*
 * Prints clipping outpute warning.
 */
void print_clipping_warning();

/*
 * Exit with exit code `exit_code` and print info.
 */
void exit_with_info(int exit_code);

#endif
