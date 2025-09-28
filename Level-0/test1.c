#include<stdio.h>
#include<string.h>
int main(){
    char a[100];
    char b[]="Dian";
    
    char d[]="Quit";
    
    for(int i=0;i>=0;i++){
        printf("Please erter:");
        scanf("%s",a);
        if(strcmp(a,b)==0){
        printf("2002\n");
        }else if(strcmp(a,b)!=0&&strcmp(a,d)!=0){
        printf("ERROR\n");
        }else if(strcmp(a,d)==0){
        break;
        }
    }

    return 0;
}