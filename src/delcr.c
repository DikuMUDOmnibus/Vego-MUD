#include <stdio.h>
main()
{
char ch;
while ((ch=getchar())!=EOF)
  if (ch!='\r')
    putchar(ch);
}
