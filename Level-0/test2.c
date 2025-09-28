#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 100
#define MAX_KEY_LENGTH 11
#define MAX_VALUE_LENGTH 11

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} KeyValue;

int main() {
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        printf("Error: Cannot open data.txt file\n");
        return 1;
    }
    
    KeyValue entries[1000];  // 存储键值对的数组
    int entryCount = 0;      // 有效条目计数
    char line[MAX_LINE_LENGTH];
    
    // 首先读取和解析所有数据
    printf("Reading data.txt...\n");
    while (fgets(line, sizeof(line), file)) {
        // 去除换行符
        line[strcspn(line, "\n")] = 0;
        
        // 跳过空行
        if (strlen(line) == 0) {
            continue;
        }
        
        // 查找冒号位置
        char *colon = strchr(line, ':');
        if (colon == NULL) {
            continue; // 跳过没有冒号的无效行
        }
        
        // 分割键和值
        *colon = '\0';
        char *keyPart = line;
        char *valuePart = colon + 1;
        
        // 去除键和值前后的空格
        while (*keyPart == ' ') keyPart++;
        while (*valuePart == ' ') valuePart++;
        
        // 去除键的尾部空格
        char *end = keyPart + strlen(keyPart) - 1;
        while (end > keyPart && *end == ' ') {
            *end = '\0';
            end--;
        }
        
        // 去除值的尾部空格
        end = valuePart + strlen(valuePart) - 1;
        while (end > valuePart && *end == ' ') {
            *end = '\0';
            end--;
        }
        
        // 验证键的有效性（不含空格，最大10个字符）
        int valid = 1;
        if (strlen(keyPart) == 0 || strlen(keyPart) > 10) {
            valid = 0;
        }
        for (int i = 0; keyPart[i] != '\0'; i++) {
            if (keyPart[i] == ' ') {
                valid = 0;
                break;
            }
        }
        
        // 验证值的长度（最大10个字符）
        if (strlen(valuePart) > 10) {
            valid = 0;
        }
        
        if (valid && entryCount < 1000) {
            // 检查重复键
            int duplicate = 0;
            for (int i = 0; i < entryCount; i++) {
                if (strcmp(entries[i].key, keyPart) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            
            if (!duplicate) {
                strcpy(entries[entryCount].key, keyPart);
                strcpy(entries[entryCount].value, valuePart);
                entryCount++;
            }
        }
    }
    
    fclose(file);
    
    printf("Data loaded successfully. Found %d valid entries.\n", entryCount);
    printf("Enter key to search (type 'Quit' to exit):\n");
    
    // 交互式查询循环
    char input[100];
    while (1) {
        printf("> ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // 去除换行符
        input[strcspn(input, "\n")] = 0;
        
        // 去除输入的前后空格
        char *start = input;
        while (*start == ' ') start++;
        char *end = start + strlen(start) - 1;
        while (end > start && *end == ' ') {
            *end = '\0';
            end--;
        }
        
        // 检查退出命令
        if (strcmp(start, "Quit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        
        // 跳过空输入
        if (strlen(start) == 0) {
            continue;
        }
        
        // 搜索键
        int found = 0;
        for (int i = 0; i < entryCount; i++) {
            if (strcmp(entries[i].key, start) == 0) {
                printf("%s\n", entries[i].value);
                found = 1;
                break;
            }
        }
        
        if (!found) {
            printf("Error\n");
        }
    }
    
    return 0;
}