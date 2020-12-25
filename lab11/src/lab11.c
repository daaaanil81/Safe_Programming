#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int isPasswordOk()
{
    char password[12];
    gets(password);
    return 0 == strcmp(password, "goodpass");
}

int main(int argc, char* argv[])
{
    int status;
    puts("Enter password: ");
    status = isPasswordOk();
    if (!status)
    {
        puts("Access denied");
        exit(-1);
    }
    puts("Passed");
    return 0;
}
