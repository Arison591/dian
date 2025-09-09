#include <stdio.h>
#include<string.h>

int main() {
    FILE *file = fopen("C:\\Users\\Arison\\Desktop\\dian\\data.txt", "r");
    if(file == NULL) {
        printf("ERROR\n");
        return 1;
    }
    
    char lie[100];
    while(fgets(lie, sizeof(lie), file)){
        
        for(int i=0;lie[i]!='\0';i++){
            if(lie[i]==':'){
                lie[i]='\0';
                char*part1=lie;
                
                char*part2=lie+(i+1);
                
                char str[100];
                for(int i=0;i>=0;i++){
                    gets(str);
                    if(strcmp(str,part1)==0){
                    printf(":%s",part2);
                    break;
                    }else if(strcmp(str,"Quit")==0){
                    goto here;
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
