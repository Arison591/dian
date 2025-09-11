#include<stdio.h>
#include<string.h>
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(){
    int a;//a用于表示楼层
    int b;//b用于表示列
    int c;//c用于表示行
    int map[5][4][4]={0};
    char str[100];
    char str1[100];
    
    for( a=0;a<5;a++){
        printf("\n-%d-\n",a+1);
        for( b=0;b<4;b++){
            for( c=0;c<4;c++){
                printf("%d",map[a][b][c]);
            }
            printf("\n");
        }
    }
    here:
    printf("Friday Floor ");
    scanf("%d",&a);
    printf("\n-%d-\n",a);
    for(b=0;b<4;b++){
        for(c=0;c<4;c++){
            printf("%d",map[a-1][b][c]);
        }
        printf("\n");
    }
    printf("Reserve Monday Floor ");
    scanf("%d",&a);
    printf("Seat ");
    scanf("%d %d",&b,&c);
    map[a-1][b-1][c-1]=2;
    printf("Friday Floor ");
    scanf("%d",&a);
    clearInputBuffer(); // 清理缓冲区
    printf("\n-%d-\n",a);
    for(b=0;b<4;b++){
        for(c=0;c<4;c++){
            printf("%d",map[a-1][b][c]);
        }
        printf("\n");
    }
    
    gets(str);
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
    
    //继续预约系统
    printf("Enter 'Continue' to make another reservation: ");
    gets(str1);
    if(strcmp(str1,"Continue")==0){
        goto here;
    }
    return 0;
}
//建立输出每层楼的预约情况的函数?
//记住，所有数组都是从零开始
//还要建立二次预约系统
