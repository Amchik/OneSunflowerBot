#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

int main() {
  srand(time(0));
  setlocale(LC_ALL, "");

  puts("Hello, world!");

  return(0);
}
