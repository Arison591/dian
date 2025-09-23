#include <stdio.h>
#include <string.h>
void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
int Enter(int isPrime)
{
    char str[100];
    

    for (int i = 0; i >= 0; i++)
    {
        printf("Please enter the user:");
        gets(str);
        if (strcmp(str, "A") == 0)
        {
            printf("Login successful.\n");
            printf("Enter OK, Exit, or Quit.\n");
            char str1[100];
            gets(str1);
            if (strcmp(str1, "Exit") == 0)
            {
                continue;
            }
            else if (strcmp(str1, "Quit") == 0)
            {
                goto there;
            }
            else if (strcmp(str1, "OK") == 0)
            {
                goto there;
            }
        }
        else if (strcmp(str, "Quit") == 0)
        {
            break;
        }
        else if (strcmp(str, "A") != 0 && strcmp(str, "admin") != 0)
        {
            printf("Error,try again.\n");
            continue;
        }
        else if (strcmp(str, "admin") == 0)
        {
            printf("Login successful.\n");
            printf("Enter OK, Exit, or Quit.\n");
            char str1[100];
            gets(str1);
            if (strcmp(str1, "Exit") == 0)
            {
                continue;
            }
            else if (strcmp(str1, "Quit") == 0)
            {
                goto there;
            }
            else if (strcmp(str1, "OK") == 0)
            { // 输入OK表示登录完毕
                goto there;
                isPrime = 1;
            }
        }
    }
there:
    return isPrime;
}

int main()
{
    int isPrime = 0;
    Enter(isPrime);
    int a; // a用于表示楼层
    int b; // b用于表示列
    int c; // c用于表示行
    int map[5][4][4] = {0};
    char str[100];
    char str1[100];

// for( a=0;a<5;a++){
//     printf("\n-%d-\n",a+1);
//     for( b=0;b<4;b++){
//         for( c=0;c<4;c++){
//             printf("%d",map[a][b][c]);
//         }
//         printf("\n");
//     }
// }
here:
    printf("Friday Floor ");//用户查询座位
    scanf("%d", &a);
    printf("\n-%d-\n", a);
    for (b = 0; b < 4; b++)
    {
        for (c = 0; c < 4; c++)
        {
            printf("%d", map[a - 1][b][c]);
        }
        printf("\n");
    }
    if (isPrime == 0)
    {
        printf("Reserve Monday Floor ");
        scanf("%d", &a);
        printf("Seat ");
        scanf("%d %d", &b, &c);
        map[a - 1][b - 1][c - 1] = 2;
        printf("Friday Floor ");
        scanf("%d", &a);
        clearInputBuffer(); // 清理缓冲区
        printf("\n-%d-\n", a);
        for (b = 0; b < 4; b++)
        {
            for (c = 0; c < 4; c++)
            {
                printf("%d", map[a - 1][b][c]);
            }
            printf("\n");
        }
    }
    if(isPrime==1){
        printf("Friday Floor ");
        scanf("%d", &a);
        clearInputBuffer(); // 清理缓冲区
        printf("\n-%d-\n", a);//管理员查询每一层的座位
        for (b = 0; b < 4; b++)
        {
            for (c = 0; c < 4; c++)
            {
                if(map[a-1][b][c]==0){
                    printf("%d",map[a-1][b][c]);
                }else if(map[a-1][b][c]==2){
                    printf("%d",1);
                }
                
            }
            printf("\n");
        }
        gets(str);//查询所有的预约座位
        if(strcmp(str,"Reservation")==0){
            for( a=0;a<5;a++){
                for( b=0;b<4;b++){
                    for( c=0;c<4;c++){
                        if(map[a][b][c]!=0){
                            printf("Friday Floor %d Seat %d %d\n",a+1,b+1,c+1);
                            
                        }
                    }
                }
            }
        }
    }

    // 继续预约系统
    printf("Enter 'Continue' to make another reservation: ");
    gets(str1);
    if (strcmp(str1, "Continue") == 0)
    {
        goto here;
    }
    return 0;
}

