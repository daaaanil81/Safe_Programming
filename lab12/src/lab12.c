#include<stdio.h>

void printMe(const char * ptr);

int main() {
 char a[10];
 scanf("%s", a);
 void(*ptrPrint)(const char*) = printMe;
 printMe (a);
 ptrPrint (a);
 return 0;
}


void printMe (const char * ptr) {
 printf("%s", ptr);
};