#include <stdio.h>
#include<string.h>

int main() {//读文件
    FILE *file = fopen("C:\\Users\\Arison\\Desktop\\dian\\data.txt", "r");
    if(file == NULL) {//如果是空文件，及时返回
        printf("ERROR\n");
        return 1;
    }
    
    char lie[100];
    while(fgets(lie, sizeof(lie), file)){//读每一行的字符串
        
        for(int i=0;lie[i]!='\0';i++){
            if(lie[i]==':'){
                lie[i]='\0';//开始分割字符串
                char*part1=lie;
                
                char*part2=lie+(i+1);
                char str[100];
                for(int i=0;i>=0;i++){//进行查询操作，这里使用for循环，防止读漏行
                    gets(str);
                    if(strcmp(str,part1)==0){
                    printf(":%s",part2);
                    break;
                    }else if(strcmp(str,"Quit")==0){
                    goto here;//一次性跳出循环
                    }else{
                    printf("ERROR\n");
                    }
                }
               
            }
        }
    }
    here:
    
    fclose(file);
    return 0;
}
