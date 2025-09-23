#include <stdio.h>
#include <string.h>
#include<time.h>
//增加了管理员登记用户临时签离开的情况
//增加了管理员强制清除不按时返回的用户座位的功能
//增加FCFS算法优化预约系统

typedef struct 
{
    int start_hour;
    int end_hour;
}Time;
typedef struct{
    int user_id;
    time_t reserve_time;
    Time time1;
}Request_inform;
typedef struct{
    Request_inform reservation;
    int is_occupied;
}Seat;
Seat map[5][4][4];

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
void readData(){//定义读取数据的函数
    FILE *file=fopen("Reservations.bin","rb");
    if(file==NULL){
        for (int f = 0; f < 5; f++) {
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    map[f][r][c].is_occupied = 0;
                    memset(&map[f][r][c].reservation, 0, sizeof(Request_inform));
                }
            }
        }
        return;
    }
    
    fread(map,sizeof(Seat),5*4*4,file);
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
void writeData(){//储存数据的函数
    FILE *file=fopen("Reservations.bin","wb");
    fwrite(map,sizeof(Seat),5*4*4,file);
    fclose(file);
}
void clear(){//clear函数
    FILE *file=fopen("Reservations.bin","wb");
    for(int f=0;f<5;f++){
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                map[f][r][c].is_occupied = 0;
                memset(&map[f][r][c].reservation, 0, sizeof(Request_inform));
            }
        }
    }
    fwrite(map,sizeof(Seat),5*4*4,file);
    fclose(file);
    printf("clear successful");
}
int TimeConflict(Time time1,Time time2){
    return(time1.start_hour<time2.end_hour&&time1.end_hour>time2.start_hour);
}
void formatTime(Time time,char*buffer){
    sprintf(buffer,"%02d:00-%02d:00", time.start_hour, time.end_hour);
}

// 自动清理所有过期的预约
void cleanupAllExpiredReservations(Seat map[5][4][4]) {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                
                
                if (map[f][r][c].is_occupied) {
                    // 如果当前时间超过预约结束时间，自动清除
                    if (current_time->tm_hour >= map[f][r][c].reservation.time1.end_hour) {
                        map[f][r][c].is_occupied = 0;
                    }
                }
            }
        }
    }
    writeData();
}

int canReserveSeat(Seat *seat, Time new_slot) {
    // 如果座位完全空闲，可以直接预约
    if (!seat->is_occupied) {
        return 1;
    }
    
    // 如果座位已被预约，检查时间段是否冲突
    Time existing_slot = seat->reservation.time1;
    
    // 检查时间段是否重叠
    int has_conflict = (new_slot.start_hour < existing_slot.end_hour && 
                        new_slot.end_hour > existing_slot.start_hour);
    
    return !has_conflict; // 无冲突返回1，有冲突返回0
}

void reserveSeat(int floor,int row,int c, int user_id){//预约座位的函数
    Time time1;//预约约时间段
    printf("Enter start hour (0-23): ");
    scanf("%d", &time1.start_hour);
    printf("Enter end hour (0-23): ");
    scanf("%d", &time1.end_hour);
    clearInputBuffer();
    
    char time_str[20];
    formatTime(map[floor][row][c].reservation.time1, time_str);
    
        
    if (map[floor][row][c].is_occupied && TimeConflict(time1, map[floor][row][c].reservation.time1)) {
            char time_str[20];
            formatTime(map[floor][row][c].reservation.time1, time_str);
            printf("Time conflict with seat at floor %d, row %d, col %d (%s)\n",floor+1, row+1, c+1, time_str);
            return;
        }else{
            map[floor][row][c].is_occupied=user_id;
            map[floor][row][c].reservation.user_id=user_id;
            map[floor][row][c].reservation.time1=time1;
            map[floor][row][c].reservation.reserve_time=time(NULL);
            void writeData();
            printf("Reserve successful");
        }
    
}    
void Leave(Seat map[5][4][4]){//签到暂时离开的函数
    int i;
    printf("Please enter the user's id:");
    scanf("%d",&i);
    clearInputBuffer();
    for (int floor = 0; floor < 5; floor++) {
        for (int row = 0; row < 4; row++) {
            for (int c = 0; c < 4; c++) {
                if (map[floor][row][c].is_occupied == i) {
                    map[floor][row][c].is_occupied = -i;
                }
            }
        }
    }
    void writeData();
}
void Return(Seat map[5][4][4]){//签到回来的函数
    int i;
    printf("Please enter the user's id:");
    scanf("%d",&i);
    clearInputBuffer();
    for (int floor = 0; floor < 5; floor++) {
        for (int row = 0; row < 4; row++) {
            for (int c = 0; c < 4; c++) {
                if (map[floor][row][c].is_occupied == -i) {
                    map[floor][row][c].is_occupied = i;
                }
            }
        }
    }
    void writeData();
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

void displayFloor(int floor) {//定义查询一层楼的函数
    printf("\nFloor %d:\n", floor);
    for (int row = 0; row < 4; row++) {
        for (int c = 0; c < 4; c++) {
            printf("%d ", map[floor-1][row][c].is_occupied);
        }
        printf("\n");
    }
}

void displayAllReservations() {//属于管理员的函数，查询所有座位
    printf("\nAll reservations:\n");
    for (int floor = 0; floor < 5; floor++) {
        for (int row = 0; row < 4; row++) {
            for (int c = 0; c < 4; c++) {
                if (map[floor][row][c].is_occupied != 0) {
                    char time_str[20];
                    formatTime(map[floor][row][c].reservation.time1,time_str);
                    printf("Floor %d, Seat %d-%d (User %d)for%s\n", 
                           floor+1, row+1, c+1, map[floor][row][c].is_occupied,time_str);
                }
            }
        }
    }
}

int main() {    
    int userCounter = 1; // 用于分配用户ID
    int isAdmin = 0;
    Enter(isAdmin,&userCounter);
    readData();//读取座位号
    cleanupAllExpiredReservations(map);
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
                    
                    displayFloor(floor);//调用查询座位函数
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
                    if (map[floor-1][row-1][c-1].is_occupied == 0) {
                        reserveSeat(floor,row,c, currentUserId);
                    } else {
                        printf("Seat already occupied by user %d.\n", map[floor-1][row-1][c-1].is_occupied);
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
            printf("\nOptions: View floor, View reservations, Clear,Re-login, Quit,Leave,Return,clearLeave\n");
            printf("Enter command: ");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "View floor") == 0) {//查询一个楼的座位
                readData();
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();//清除换行符
                
                if (floor >= 1 && floor <= 5) {
                    printf("\nFloor %d (0=available, number=user ID):\n", floor);
                    displayFloor(floor);
                } else {
                    printf("Invalid floor number.\n");
                }
            }else if(strcmp(command, "View reservations") == 0) {
                displayAllReservations();//调用管理员专属函数
            }else if(strcmp(command, "Quit") == 0) {
                break;
            }else if(strcmp(command, "Re-login") == 0) {
                // 切换到另一个用户
                isAdmin = Enter(isAdmin,&userCounter);
                if (isAdmin == -1) break;
                currentUserId = isAdmin ? 0 : userCounter++;
            }else if(strcmp(command, "Clear") == 0){
                clear(map,&userCounter);//清除数据
            }else if(strcmp(command, "Leave") == 0){
                Leave(map);//执行临时签到离开操作
            }else if(strcmp(command, "Return") == 0){
                Return(map);//执行签到回来操作
            }else if(strcmp(command, "clearLeave") == 0){
                void cleanupAllExpiredReservations();//执行清除过期签到离开操作
            }else {
                printf("Unknown command.\n");
            }
        }
    }

    printf("Thank you for using the reservation system.\n");
    return 0;
}
