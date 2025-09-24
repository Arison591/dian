#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// 时间结构体（小时级，支持0-23）
typedef struct {
    int start_hour;  // 开始小时
    int end_hour;    // 结束小时
} Time;

// 单个预约时段节点
typedef struct ReservationNode {
    Time time_slot;              // 预约时间段
    time_t reserve_time;         // 预约提交时间
    time_t leave_time;           // 临时离开时间（0表示未离开）
    int user_id;                 // 用户ID
    struct ReservationNode* next; // 下一个预约节点
} ReservationNode;

// 座位结构体（支持多时段预约）
typedef struct {
    ReservationNode* reservations; // 预约链表头指针
    int current_user;             // 当前使用者ID（0表示空闲）
    int is_occupied;              // 当前状态：0=空闲，1=占用，-1=临时离开
} Seat;

// 全局座位地图：5层楼，每层4行4列（共80个座位）
Seat seat_map[5][4][4];
// 全局用户计数器（记录已分配的最大用户ID，从1开始）
int user_counter = 1;
// 管理员密码
#define ADMIN_PASSWORD "lib_admin123"
// 临时离开超时时间（单位：秒，30分钟=1800秒）
#define LEAVE_TIMEOUT 1800

// 清除输入缓冲区
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 格式化时间段为字符串
void format_time(Time time, char* buffer) {
    sprintf(buffer, "%02d:00-%02d:00", time.start_hour, time.end_hour);
}

// 判断两个时间段是否冲突
int is_time_conflict(Time t1, Time t2) {
    return (t1.start_hour < t2.end_hour) && (t1.end_hour > t2.start_hour);
}

// 验证时间段合法性
int is_time_valid(Time time) {
    return (time.start_hour >= 0 && time.start_hour < 23) 
        && (time.end_hour > time.start_hour && time.end_hour <= 23);
}

// 验证预约时长合法性（最长4小时）
int is_duration_valid(Time time) {
    return (time.end_hour - time.start_hour) <= 4;
}

// 创建新的预约节点
ReservationNode* create_reservation_node(int user_id, Time time_slot) {
    ReservationNode* new_node = (ReservationNode*)malloc(sizeof(ReservationNode));
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    new_node->user_id = user_id;
    new_node->time_slot = time_slot;
    new_node->reserve_time = time(NULL);
    new_node->leave_time = 0;
    new_node->next = NULL;
    
    return new_node;
}

// 检查时间段是否与座位所有预约冲突
int is_seat_available(Seat* seat, Time new_slot) {
    ReservationNode* current = seat->reservations;
    while (current != NULL) {
        if (is_time_conflict(current->time_slot, new_slot)) {
            return 0; // 冲突
        }
        current = current->next;
    }
    return 1; // 可用
}

// 释放整个预约链表
void free_reservation_list(ReservationNode* head) {
    ReservationNode* current = head;
    while (current != NULL) {
        ReservationNode* next = current->next;
        free(current);
        current = next;
    }
}

// 数据持久化：保存座位信息到文件
void save_seat_data() {
    FILE* file = fopen("Library_Seats_Multi.bin", "wb");
    if (file == NULL) {
        printf("Failed to save seat data!\n");
        return;
    }
    
    // 保存每个座位的基本信息和预约数量
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                
                // 保存座位基本信息
                fwrite(&seat->current_user, sizeof(int), 1, file);
                fwrite(&seat->is_occupied, sizeof(int), 1, file);
                
                // 保存预约数量
                int reservation_count = 0;
                ReservationNode* current = seat->reservations;
                while (current != NULL) {
                    reservation_count++;
                    current = current->next;
                }
                fwrite(&reservation_count, sizeof(int), 1, file);
                
                // 保存每个预约节点
                current = seat->reservations;
                while (current != NULL) {
                    fwrite(current, sizeof(ReservationNode), 1, file);
                    current = current->next;
                }
            }
        }
    }
    
    fclose(file);
}

// 数据加载：从文件读取座位信息
void load_seat_data() {
    FILE* file = fopen("Library_Seats_Multi.bin", "rb");
    if (file == NULL) {
        // 首次运行，初始化所有座位
        for (int f = 0; f < 5; f++) {
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    seat_map[f][r][c].reservations = NULL;
                    seat_map[f][r][c].current_user = 0;
                    seat_map[f][r][c].is_occupied = 0;
                }
            }
        }
        return;
    }
    
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                seat->reservations = NULL;
                
                // 读取座位基本信息
                fread(&seat->current_user, sizeof(int), 1, file);
                fread(&seat->is_occupied, sizeof(int), 1, file);
                
                // 读取预约数量
                int reservation_count;
                fread(&reservation_count, sizeof(int), 1, file);
                
                // 读取每个预约节点
                ReservationNode* last = NULL;
                for (int i = 0; i < reservation_count; i++) {
                    ReservationNode* new_node = (ReservationNode*)malloc(sizeof(ReservationNode));
                    fread(new_node, sizeof(ReservationNode), 1, file);
                    new_node->next = NULL;
                    
                    if (seat->reservations == NULL) {
                        seat->reservations = new_node;
                    } else {
                        last->next = new_node;
                    }
                    last = new_node;
                }
            }
        }
    }
    
    fclose(file);
}

// 保存用户计数器
void save_user_counter() {
    FILE* file = fopen("Library_UserCounter.bin", "wb");
    if (file == NULL) {
        printf("Failed to save user counter!\n");
        return;
    }
    fwrite(&user_counter, sizeof(int), 1, file);
    fclose(file);
}

// 加载用户计数器
void load_user_counter() {
    FILE* file = fopen("Library_UserCounter.bin", "rb");
    if (file == NULL) {
        user_counter = 1;
        return;
    }
    fread(&user_counter, sizeof(int), 1, file);
    fclose(file);
}

// 自动清理过期预约
void auto_clean_expired() {
    time_t now = time(NULL);
    struct tm* current_tm = localtime(&now);
    int cleaned_count = 0;
    
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                ReservationNode** current = &seat->reservations;
                
                while (*current != NULL) {
                    ReservationNode* node = *current;
                    
                    // 检查是否过期（当前时间超过预约结束时间）
                    if (current_tm->tm_hour >= node->time_slot.end_hour) {
                        // 删除过期节点
                        *current = node->next;
                        free(node);
                        cleaned_count++;
                    } else {
                        current = &(*current)->next;
                    }
                }
                
                // 更新座位状态
                if (seat->reservations == NULL) {
                    seat->current_user = 0;
                    seat->is_occupied = 0;
                } else {
                    // 检查当前是否有正在进行中的预约
                    int has_current = 0;
                    ReservationNode* node = seat->reservations;
                    while (node != NULL) {
                        if (current_tm->tm_hour >= node->time_slot.start_hour && 
                            current_tm->tm_hour < node->time_slot.end_hour) {
                            seat->current_user = node->user_id;
                            seat->is_occupied = (node->leave_time == 0) ? 1 : -1;
                            has_current = 1;
                            break;
                        }
                        node = node->next;
                    }
                    if (!has_current) {
                        seat->current_user = 0;
                        seat->is_occupied = 0;
                    }
                }
            }
        }
    }
    
    if (cleaned_count > 0) {
        save_seat_data();
        printf("Automatic cleaning completed: %d expired reservations deleted\n", cleaned_count);
    } else {
        printf("No expired reservations to clean\n");
    }
}

// 添加预约到座位
int add_reservation_to_seat(int floor, int row, int col, int user_id, Time time_slot) {
    Seat* seat = &seat_map[floor-1][row-1][col-1];
    
    // 验证时间段
    if (!is_time_valid(time_slot)) {
        printf("Invalid time slot!\n");
        return 0;
    }
    
    if (!is_duration_valid(time_slot)) {
        printf("Reservation duration cannot exceed 4 hours!\n");
        return 0;
    }
    
    // 检查时间冲突
    if (!is_seat_available(seat, time_slot)) {
        printf("This time slot conflicts with existing reservations!\n");
        return 0;
    }
    
    // 创建新预约节点
    ReservationNode* new_node = create_reservation_node(user_id, time_slot);
    if (new_node == NULL) {
        return 0;
    }
    
    // 插入到链表（按开始时间排序）
    if (seat->reservations == NULL || 
        time_slot.start_hour < seat->reservations->time_slot.start_hour) {
        new_node->next = seat->reservations;
        seat->reservations = new_node;
    } else {
        ReservationNode* current = seat->reservations;
        while (current->next != NULL && 
               current->next->time_slot.start_hour < time_slot.start_hour) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
    
    // 更新座位状态
    time_t now = time(NULL);
    struct tm* current_tm = localtime(&now);
    if (current_tm->tm_hour >= time_slot.start_hour && 
        current_tm->tm_hour < time_slot.end_hour) {
        seat->current_user = user_id;
        seat->is_occupied = 1;
    }
    
    printf("Reservation successful!\n");
    return 1;
}

// 多时段预约功能
void reserve_seat_multi(int user_id) {
    int floor, row, col;
    Time time_slot;
    
    printf("Enter floor (1-5): ");
    if (scanf("%d", &floor) != 1 || floor < 1 || floor > 5) {
        printf("Invalid floor input!\n");
        clear_input_buffer();
        return;
    }
    
    printf("Enter seat (row column, e.g.: 1 2): ");
    if (scanf("%d %d", &row, &col) != 2 || row < 1 || row > 4 || col < 1 || col > 4) {
        printf("Invalid seat input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    printf("Enter start time (0-22): ");
    if (scanf("%d", &time_slot.start_hour) != 1 || time_slot.start_hour < 0 || time_slot.start_hour > 22) {
        printf("Invalid start time input!\n");
        clear_input_buffer();
        return;
    }
    
    printf("Enter end time (%d-23): ", time_slot.start_hour + 1);
    if (scanf("%d", &time_slot.end_hour) != 1 || time_slot.end_hour <= time_slot.start_hour || time_slot.end_hour > 23) {
        printf("Invalid end time input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    if (add_reservation_to_seat(floor, row, col, user_id, time_slot)) {
        save_seat_data();
    }
}

// 查看指定楼层座位状态
void view_floor_status() {
    int floor;
    printf("Enter the floor to view (1-5): ");
    if (scanf("%d", &floor) != 1 || floor < 1 || floor > 5) {
        printf("Invalid floor input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    printf("\n=== Floor %d Seat Status ===\n", floor);
    printf("Row/Col |   1     2     3     4\n");
    printf("-----|-----------------------\n");
    
    for (int r = 0; r < 4; r++) {
        printf(" %d   | ", r + 1);
        for (int c = 0; c < 4; c++) {
            Seat* seat = &seat_map[floor-1][r][c];
            if (seat->is_occupied == 0) {
                printf("Free  ");
            } else if (seat->is_occupied > 0) {
                printf("U%-3d ", seat->current_user);
            } else {
                printf("L%-3d ", seat->current_user);
            }
        }
        printf("\n");
    }
    printf("===========================\n");
}

// 查看座位的所有预约
void view_seat_reservations() {
    int floor, row, col;
    
    printf("Enter seat information (floor row column, e.g.: 1 1 1): ");
    if (scanf("%d %d %d", &floor, &row, &col) != 3 || 
        floor < 1 || floor > 5 || row < 1 || row > 4 || col < 1 || col > 4) {
        printf("Invalid seat information input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    Seat* seat = &seat_map[floor-1][row-1][col-1];
    ReservationNode* current = seat->reservations;
    char time_str[20];
    int count = 0;
    
    printf("\n=== All Reservations for Seat (%d,%d) on Floor %d ===\n", row, col, floor);
    
    if (current == NULL) {
        printf("No reservations for this seat\n");
    } else {
        while (current != NULL) {
            format_time(current->time_slot, time_str);
            printf("%d. User %d | Time slot: %s | Reservation time: %s", 
                   ++count, current->user_id, time_str, ctime(&current->reserve_time));
            current = current->next;
        }
    }
    printf("===================================\n");
}

// 查看用户的所有预约
void view_user_reservations(int user_id) {
    printf("\n=== All Reservations for User %d ===\n", user_id);
    int found = 0;
    
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                ReservationNode* current = seat_map[f][r][c].reservations;
                while (current != NULL) {
                    if (current->user_id == user_id) {
                        char time_str[20];
                        format_time(current->time_slot, time_str);
                        printf("Floor %d Seat (%d,%d) | Time slot: %s\n", 
                               f+1, r+1, c+1, time_str);
                        found = 1;
                    }
                    current = current->next;
                }
            }
        }
    }
    
    if (!found) {
        printf("No reservation records\n");
    }
    printf("==========================\n");
}

// 取消用户的特定预约
void cancel_reservation(int user_id) {
    int floor, row, col;
    Time time_slot;
    
    printf("Enter the reservation information to cancel\n");
    printf("Floor (1-5): ");
    if (scanf("%d", &floor) != 1 || floor < 1 || floor > 5) {
        printf("Invalid floor input!\n");
        clear_input_buffer();
        return;
    }
    
    printf("Seat (row column): ");
    if (scanf("%d %d", &row, &col) != 2 || row < 1 || row > 4 || col < 1 || col > 4) {
        printf("Invalid seat input!\n");
        clear_input_buffer();
        return;
    }
    
    printf("Start time: ");
    if (scanf("%d", &time_slot.start_hour) != 1) {
        printf("Invalid start time input!\n");
        clear_input_buffer();
        return;
    }
    
    printf("End time: ");
    if (scanf("%d", &time_slot.end_hour) != 1) {
        printf("Invalid end time input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    Seat* seat = &seat_map[floor-1][row-1][col-1];
    ReservationNode** current = &seat->reservations;
    int canceled = 0;
    
    while (*current != NULL) {
        ReservationNode* node = *current;
        if (node->user_id == user_id &&
            node->time_slot.start_hour == time_slot.start_hour &&
            node->time_slot.end_hour == time_slot.end_hour) {
            
            // 删除节点
            *current = node->next;
            free(node);
            canceled = 1;
            printf("Reservation canceled successfully!\n");
            break;
        }
        current = &(*current)->next;
    }
    
    if (!canceled) {
        printf("No corresponding reservation record found!\n");
        return;
    }
    
    // 更新座位状态
    if (seat->reservations == NULL) {
        seat->current_user = 0;
        seat->is_occupied = 0;
    }
    
    save_seat_data();
}

// 管理员查看所有预约
void view_all_reservations() {
    printf("\n=== All Current Reservations ===\n");
    int found = 0;
    
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                ReservationNode* current = seat_map[f][r][c].reservations;
                while (current != NULL) {
                    char time_str[20];
                    format_time(current->time_slot, time_str);
                    printf("Floor %d Seat (%d,%d) | User %d | Time slot: %s\n", 
                           f+1, r+1, c+1, current->user_id, time_str);
                    found = 1;
                    current = current->next;
                }
            }
        }
    }
    
    if (!found) {
        printf("No reservation records\n");
    }
    printf("=====================\n");
}

// 临时离开登记
void user_leave(int admin_flag) {
    if (!admin_flag) {
        printf("Insufficient permissions!\n");
        return;
    }
    
    int user_id;
    printf("Enter the ID of the user to register departure: ");
    if (scanf("%d", &user_id) != 1 || user_id <= 0) {
        printf("Invalid user ID input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    int found = 0;
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                if (seat->current_user == user_id && seat->is_occupied == 1) {
                    seat->is_occupied = -1;
                    
                    // 更新对应预约节点的离开时间
                    ReservationNode* current = seat->reservations;
                    time_t now = time(NULL);
                    while (current != NULL) {
                        if (current->user_id == user_id) {
                            struct tm* current_tm = localtime(&now);
                            if (current_tm->tm_hour >= current->time_slot.start_hour && 
                                current_tm->tm_hour < current->time_slot.end_hour) {
                                current->leave_time = now;
                                break;
                            }
                        }
                        current = current->next;
                    }
                    
                    printf("User %d departure registered successfully!\n", user_id);
                    found = 1;
                }
            }
        }
    }
    
    if (found) {
        save_seat_data();
    } else {
        printf("No occupied seat found for User %d!\n", user_id);
    }
}

// 临时离开返回登记
void user_return(int admin_flag) {
    if (!admin_flag) {
        printf("Insufficient permissions!\n");
        return;
    }
    
    int user_id;
    printf("Enter the ID of the user to register return: ");
    if (scanf("%d", &user_id) != 1 || user_id <= 0) {
        printf("Invalid user ID input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    int found = 0;
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                if (seat->current_user == user_id && seat->is_occupied == -1) {
                    seat->is_occupied = 1;
                    
                    // 清空对应预约节点的离开时间
                    ReservationNode* current = seat->reservations;
                    time_t now = time(NULL);
                    while (current != NULL) {
                        if (current->user_id == user_id) {
                            struct tm* current_tm = localtime(&now);
                            if (current_tm->tm_hour >= current->time_slot.start_hour && 
                                current_tm->tm_hour < current->time_slot.end_hour) {
                                current->leave_time = 0;
                                break;
                            }
                        }
                        current = current->next;
                    }
                    
                    printf("User %d return registered successfully!\n", user_id);
                    found = 1;
                }
            }
        }
    }
    
    if (found) {
        save_seat_data();
    } else {
        printf("No temporary departure record found for User %d!\n", user_id);
    }
}

// 强制清理指定座位
void force_clear_seat(int admin_flag) {
    if (!admin_flag) {
        printf("Insufficient permissions!\n");
        return;
    }
    
    int floor, row, col;
    printf("Enter the seat to force clear (floor row column): ");
    if (scanf("%d %d %d", &floor, &row, &col) != 3 || 
        floor < 1 || floor > 5 || row < 1 || row > 4 || col < 1 || col > 4) {
        printf("Invalid seat information input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    Seat* seat = &seat_map[floor-1][row-1][col-1];
    if (seat->reservations == NULL) {
        printf("This seat is already free!\n");
        return;
    }
    
    printf("Confirm to force clear Seat (%d,%d) on Floor %d? (yes/no): ", row, col, floor);
    char confirm[10];
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';
    
    if (strcmp(confirm, "yes") == 0) {
        free_reservation_list(seat->reservations);
        seat->reservations = NULL;
        seat->current_user = 0;
        seat->is_occupied = 0;
        save_seat_data();
        printf("Force clear successful!\n");
    } else {
        printf("Operation canceled\n");
    }
}

// 管理员登录
int admin_login() {
    char password[50];
    printf("Enter admin password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';
    
    if (strcmp(password, ADMIN_PASSWORD) == 0) {
        printf("Admin login successful!\n");
        return 1;
    } else {
        printf("Incorrect password!\n");
        return 0;
    }
}

// 用户登录
int user_login() {
    char choice[10];
    printf("Do you have a user ID already? (yes/no): ");
    fgets(choice, sizeof(choice), stdin);
    choice[strcspn(choice, "\n")] = '\0';
    
    if (strcmp(choice, "yes") == 0) {
        int user_id;
        printf("Enter user ID: ");
        if (scanf("%d", &user_id) != 1 || user_id <= 0 || user_id >= user_counter) {
            printf("Invalid user ID!\n");
            clear_input_buffer();
            return -1;
        }
        clear_input_buffer();
        printf("User %d login successful!\n", user_id);
        return user_id;
    } else if (strcmp(choice, "no") == 0) {
        int new_id = user_counter++;
        save_user_counter();
        printf("Registration successful! Your user ID is: %d\n", new_id);
        return new_id;
    } else {
        printf("Please enter yes or no\n");
        return -1;
    }
}

// 主菜单
void main_menu(int is_admin, int user_id) {
    char command[20];
    
    while (1) {
        if (!is_admin) {
            // 普通用户菜单
            printf("\n===== Library Seat Reservation System (User %d) =====\n", user_id);
            printf("1. View floor status\n");
            printf("2. Reserve seat\n");
            printf("3. View my reservations\n");
            printf("4. Cancel reservation\n");
            printf("5. View all reservations for a seat\n");
            printf("6. Re-login\n");
            printf("7. Exit\n");
            printf("Please enter your choice (1-7): ");
        } else {
            // 管理员菜单
            printf("\n===== Library Seat Reservation System (Admin) =====\n");
            printf("1. View floor status\n");
            printf("2. View all reservations\n");
            printf("3. Register user departure\n");
            printf("4. Register user return\n");
            printf("5. Auto clean expired reservations\n");
            printf("6. Force clear seat\n");
            printf("7. Re-login\n");
            printf("8. Exit\n");
            printf("Please enter your choice (1-8): ");
        }
        
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        
        if (!is_admin) {
            if (strcmp(command, "1") == 0) {
                view_floor_status();
            } else if (strcmp(command, "2") == 0) {
                reserve_seat_multi(user_id);
            } else if (strcmp(command, "3") == 0) {
                view_user_reservations(user_id);
            } else if (strcmp(command, "4") == 0) {
                cancel_reservation(user_id);
            } else if (strcmp(command, "5") == 0) {
                view_seat_reservations();
            } else if (strcmp(command, "6") == 0) {
                printf("Re-logging in...\n");
                break;
            } else if (strcmp(command, "7") == 0) {
                printf("Thank you for using! Goodbye!\n");
                exit(0);
            } else {
                printf("Invalid command! Please enter 1-7\n");
            }
        } else {
            if (strcmp(command, "1") == 0) {
                view_floor_status();
            } else if (strcmp(command, "2") == 0) {
                view_all_reservations();
            } else if (strcmp(command, "3") == 0) {
                user_leave(is_admin);
            } else if (strcmp(command, "4") == 0) {
                user_return(is_admin);
            } else if (strcmp(command, "5") == 0) {
                auto_clean_expired();
            } else if (strcmp(command, "6") == 0) {
                force_clear_seat(is_admin);
            } else if (strcmp(command, "7") == 0) {
                printf("Re-logging in...\n");
                break;
            } else if (strcmp(command, "8") == 0) {
                printf("Thank you for using! Goodbye!\n");
                exit(0);
            } else {
                printf("Invalid command! Please enter 1-8\n");
            }
        }
    }
}

// 程序入口
int main() {
    // 初始化数据
    load_seat_data();
    load_user_counter();
    
    printf("Welcome to the Library Seat Reservation System!\n");
    
    while (1) {
        char user_type[10];
        printf("\nPlease select login type (user/admin): ");
        fgets(user_type, sizeof(user_type), stdin);
        user_type[strcspn(user_type, "\n")] = '\0';
        
        if (strcmp(user_type, "user") == 0) {
            int user_id = user_login();
            if (user_id != -1) {
                main_menu(0, user_id);
            }
        } else if (strcmp(user_type, "admin") == 0) {
            int is_admin = admin_login();
            if (is_admin) {
                main_menu(1, 0);
            }
        } else {
            printf("Please enter user or admin\n");
        }
    }
    
    return 0;
}
