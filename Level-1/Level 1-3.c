#include <stdio.h>
#include <string.h>


void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
void readData(int map[5][4][4]){//定义读取数据的函数
    FILE *file=fopen("Reservations.bin","rb");
    if(file==NULL){
        return;
    }
    fread(map,sizeof(int),5*4*4,file);
    fclose(file);
}
void readUser(int* userCounter){//定义读取用户id的函数
    FILE *file=fopen("User.bin","rb");
    if(file==NULL){
        return;
    }
    fread(userCounter,sizeof(int),1,file);
    fclose(file);
}
void writeUser(int userCounter){//储存用户id的函数
    FILE *file=fopen("User.bin","wb");
    fwrite(&userCounter,sizeof(int),1,file);
    fclose(file);
}
void writeData(int map[5][4][4]){//储存数据的函数
    FILE *file=fopen("Reservations.bin","wb");
    fwrite(map,sizeof(int),5*4*4,file);
    fclose(file);
}
void clear(int map[5][4][4],int *userCounter){//clear函数
    memset(map,0,sizeof(int)*5*4*4);
    *userCounter=1;
    FILE *file=fopen("Reservations.bin","wb");
    FILE *file1=fopen("User.bin","wb");
    fwrite(map,sizeof(int),5*4*4,file);
    fwrite(userCounter,sizeof(int),1,file1);
    fclose(file);
    fclose(file1);
    printf("clear successful");
}
int Enter(int isAdmin,int* userCounter) {//定义登录函数
    char username[100];
    readUser(userCounter);//每次登录，系统都读取用户数据

    while (1) {
        printf("Please enter the username (or 'Quit' to exit): ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';

        if (strcmp(username, "Quit") == 0) {
            isAdmin=-1;
            return isAdmin;
            
        }
        else if (strcmp(username, "A") == 0) {
            printf("User login successful.\n");
            isAdmin = 0;
            break;
        }
        else if (strcmp(username, "admin") == 0) {
            printf("Admin login successful.\n");
            isAdmin = 1;
            break;
        }
        else {
            printf("Error, try again.\n");
        }
    }


    char command[100];
    while (1) {
        printf("Enter OK to continue, Exit to re-login, or Quit to exit: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "OK") == 0) {
            return isAdmin;
            
        }
        else if (strcmp(command, "Exit") == 0) {
            return Enter( isAdmin,userCounter);
        }
        else if (strcmp(command, "Quit") == 0) {
            isAdmin=-1;
            return isAdmin;
            
        }
        else {
            printf("Error command.\n");
        }
    }
}

void displayFloor(int map[5][4][4], int floor) {//定义查询一层楼的函数
    printf("\nFloor %d:\n", floor);
    for (int row = 0; row < 4; row++) {
        for (int c = 0; c < 4; c++) {
            printf("%d ", map[floor-1][row][c]);
        }
        printf("\n");
    }
}

void displayAllReservations(int map[5][4][4]) {//属于管理员的函数，查询所有座位
    printf("\nAll reservations:\n");
    for (int floor = 0; floor < 5; floor++) {
        for (int row = 0; row < 4; row++) {
            for (int c = 0; c < 4; c++) {
                if (map[floor][row][c] != 0) {
                    printf("Floor %d, Seat %d-%d (User %d)\n", 
                           floor+1, row+1, c+1, map[floor][row][c]);
                }
            }
        }
    }
}

int main() {
    int map[5][4][4] = {0};
    int userCounter = 1; // 用于分配用户ID
    int isAdmin = 0;
    Enter(isAdmin,&userCounter);
    readData(map);//读取座位号
    
    if (isAdmin == -1) {
        printf("Program exited.\n");
        return 0;
    }

    char command[100];
    
    readUser(&userCounter);//读取用户id
    int currentUserId = isAdmin ? 0 : userCounter++; // 管理员ID为0，普通用户从1开始
    writeUser(userCounter);//读入用户id
    while (1) {
        
        if (isAdmin == 0) { // 用户
            printf("\nOptions: View floor, Reserve seat, Re-login, Quit\n");
            printf("Enter command: ");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';
            if (strcmp(command, "View floor") == 0) {
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();//清除\n
                
                if (floor >= 1 && floor <= 5) {
                    
                    displayFloor(map, floor);//调用查询座位函数
                } else {
                    printf("Invalid floor number.\n");
                }
            }
            else if(strcmp(command, "Reserve seat") == 0) {
                int floor, row, c;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                printf("Enter seat (row column, e.g., 1 2): ");
                scanf("%d %d", &row, &c);
                clearInputBuffer();//清除                
                if (floor >= 1 && floor <= 5 && row >= 1 && row <= 4 && c >= 1 && c <= 4) {
                    if (map[floor-1][row-1][c-1] == 0) {
                        map[floor-1][row-1][c-1] = currentUserId; // 使用用户ID标记预约
                        writeData(map);//读入座位号
                        printf("Reservation successful. Your user ID: %d\n", currentUserId);
                        displayFloor(map, floor);
                    } else {
                        printf("Seat already occupied by user %d.\n", map[floor-1][row-1][c-1]);
                    }
                } else {
                    printf("Invalid input.\n");
                }
            }
            else if(strcmp(command, "Quit") == 0) {
                break;
            }else if(strcmp(command, "Re-login") == 0) {
                // 切换到另一个用户
                isAdmin = Enter(isAdmin,&userCounter);
                if (isAdmin == -1) break;
                currentUserId = isAdmin ? 0 : userCounter++;
            }
            else {
                printf("Unknown command.\n");
            }
        }
        else { // 管理员
            printf("\nOptions: View floor, View reservations, Clear,Re-login, Quit\n");
            printf("Enter command: ");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "View floor") == 0) {//查询一个楼的座位
                readData(map);
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();//清除换行符
                
                if (floor >= 1 && floor <= 5) {
                    printf("\nFloor %d (0=available, number=user ID):\n", floor);
                    for (int row = 0; row < 4; row++) {
                        for (int c = 0; c < 4; c++) {
                            printf("%d ", map[floor-1][row][c]);
                        }
                        printf("\n");
                    }
                } else {
                    printf("Invalid floor number.\n");
                }
            }else if(strcmp(command, "View reservations") == 0) {
                displayAllReservations(map);//调用管理员专属函数
            }else if(strcmp(command, "Quit") == 0) {
                break;
            }else if(strcmp(command, "Re-login") == 0) {
                // 切换到另一个用户
                isAdmin = Enter(isAdmin,&userCounter);
                if (isAdmin == -1) break;
                currentUserId = isAdmin ? 0 : userCounter++;
            }else if(strcmp(command, "Clear") == 0){
                clear(map,&userCounter);//清除数据
            }
            else {
                printf("Unknown command.\n");
            }
        }
    }

    printf("Thank you for using the reservation system.\n");
    return 0;
}
