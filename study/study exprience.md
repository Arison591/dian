* # 前言：本文档用于记录记录完成任务过程中的各种问题及心得等，本人编程基础约为0，属于典型现学型选手，纵使不能加入dian团队，也相信会受益匪浅。

* # TEST 1：

   完成时间：2025.9.8，历时2days.

   Test 1的编程感觉很简单，但是在编译过程中也遇到了一些问题并学到了一些东西.

   在test 1的完成过程中，我了解到除了*stdio.h*以外的头文件如*string.h*和*stdlib.h*，其中由于使用字符串数组，我了解了*string.h*，让编程中的字符串有了更多可操作性，同时顺带了解了*stdlib.h*,并学会了申请内存等基本操作.

   当然在此也遇到了一些问题，比如1.以前我惯用scanf后加上&，但是后来了解到，字符串数组本身就自带指针，不需&.

   2.同时，我学到了字符串的等于并非简单的==，而是应该引用库函数strcmp（），当然在此过程遇到了问题，

   3.比如![alt text](image.png)，后来看到别的编程教学了解到要加上==0，至于原因，稍后探寻。
   
   4.还有遇到了逻辑错误，比如![alt text](image-1.png)，这个错误导致了无限循环，后来便把执行动作放在了循环里面。Test 1且到这里，还有疑问，应该会在后续补档。

   5.整点活吧，能不能引入动态内存呢？



补档：strcmp加上==0，是因为函数返回值是0.    记于2025.9.10.0：05
* # Test 2:

  
  完成时间2025.9.9，历时1day。

  Test 2的编程感觉难度有所提升，我也去学习了一些东西。首先我在对题目的理解上有一些偏差，后来在室友的提点下，才开始学习字符串分割，文件读写等相关知识。我对此的直观感觉就是，文件读写和字符串分割很好上手，但是想用熟练还是需要大量练习，我记了很多笔记，比如学习了库函数fgets，get，fopen，fclose，以及strcpy，strcat，strcmp，strstr，strcspn等，在敲代码过程中也以笔记作了参考.

  我的思路是把文件全部读下来，每一行字符串都做一个分割，然后输入的字符串与每行第一个字符串比较，相同就输出第二个字符串。最后的完整代码是![alt text](image-2.png)，当然在这个过程中遇到了问题。比如：1.我一开始疑惑fgets为什么要放在while里面，后来发现是由于fgets一次只能读一行。

  2.再次，在分割字符串的时候，我采用了指针，却疑惑在printf时为什么不用*调用指针，请教了ai才发现了%s与%c的区别。

  3.同时我还发现一个问题，就是这个程序必须按行依次输入键才会输出相应的值，有没有办法输入一个键就能输出一个值而不需要按照顺序呢？这个问题留给后面吧。

* # Level 1-1

完成时间：2025.9.10

我觉得这个小任务不算难吧...思路也比较清晰，使用continue来重复循环达到Exit的目的，当然我感觉我的代码有点怪怪的，似乎条件分支有点混乱，用了好多重复的条件，用while会不会好一点？这个问题留在这后续再看吧.

在过程中遇到了一点问题：![alt text](image-3.png)也是在这里我意识到strcmp函数是字符串与字符串的比较，而‘’只是字符，要加上""成为字符串.
这一步就这么多。
  

  * # Level 1-2



原初代码是
```c
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