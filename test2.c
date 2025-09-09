#include <stdio.h>

int main() {
    FILE *file = fopen("C:\\Users\\Arison\\Desktop\\dian\\data.txt", "r");
    if(file == NULL) {
        printf("ERROR\n");
        return 1;
    }
    
    char line[100];
    while(fgets(line, sizeof(line), file)){
        printf("%s", line);
    }
    
    fclose(file);
    return 0;
}
