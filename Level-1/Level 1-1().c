#include<stdio.h>
#include<string.h>
int main(){
    char str[100];
    
    for(int i=0;i>=0;i++){
        printf("Please enter the user:");
        gets(str);
        if(strcmp(str,"A")==0){
            printf("Login successful.\n");
            printf("Enter OK, Exit, or Quit.\n");
            char str1[100];
            gets(str1);
            if(strcmp(str1,"Exit")==0){
                continue;
            }else if(strcmp(str1,"Quit")==0){
                goto here;
            }else if(strcmp(str1,"OK")==0){
                goto here;
            }
        }else if(strcmp(str,"Quit")==0){
            break;
        }else if(strcmp(str,"A")!=0&&strcmp(str,"admin")!=0){
            printf("Error,try again.\n");
            continue;
        }else if(strcmp(str,"admin")==0){
            printf("Login successful.\n");
            printf("Enter OK, Exit, or Quit.\n");
            char str1[100];
            gets(str1);
            if(strcmp(str1,"Exit")==0){
                continue;
            }else if(strcmp(str1,"Quit")==0){
                goto here;
            }else if(strcmp(str1,"OK")==0){//输入OK表示登录完毕
                goto here;
            }
        }
    }    here:
    return 0;
}