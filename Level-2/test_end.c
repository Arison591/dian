#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// 时间结构体（小时级，支持0-23）
typedef struct {
    int start_hour;  // 开始小时
    int end_hour;    // 结束小时
} Time;

// 预约请求信息结构体
typedef struct {
    int user_id;          // 用户ID
    time_t reserve_time;  // 预约提交时间（时间戳）
    Time time_slot;       // 预约时间段
    time_t leave_time;    // 临时离开时间（0表示未离开）
} RequestInfo;

// 座位结构体
typedef struct {
    RequestInfo reservation;  // 预约信息
    int is_occupied;          // 占用状态：0=空闲，正整数=用户ID，负整数=临时离开（绝对值为用户ID）
} Seat;

// 全局座位地图：5层楼，每层4行4列（共80个座位）
Seat seat_map[5][4][4];
// 全局用户计数器（记录已分配的最大用户ID，从1开始）
int user_counter = 1;
// 管理员密码（可自定义修改）
#define ADMIN_PASSWORD "lib_admin123"
// 临时离开超时时间（单位：秒，此处设30分钟=1800秒）
#define LEAVE_TIMEOUT 1800

// 清除输入缓冲区（解决scanf后fgets读取残留换行问题）
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 格式化时间段为字符串（如"10:00-12:00"）
void format_time(Time time, char* buffer) {
    sprintf(buffer, "%02d:00-%02d:00", time.start_hour, time.end_hour);
}

// 数据持久化：保存座位信息到二进制文件
void save_seat_data() {
    FILE* file = fopen("Library_Seats.bin", "wb");
    if (file == NULL) {
        printf("Error: Failed to save seat data!\n");
        return;
    }
    fwrite(seat_map, sizeof(Seat), 5 * 4 * 4, file);
    fclose(file);
}

// 数据加载：从二进制文件读取座位信息
void load_seat_data() {
    FILE* file = fopen("Library_Seats.bin", "rb");
    if (file == NULL) {
        // 首次运行无文件，初始化座位为空闲
        for (int f = 0; f < 5; f++) {
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    seat_map[f][r][c].is_occupied = 0;
                    memset(&seat_map[f][r][c].reservation, 0, sizeof(RequestInfo));
                }
            }
        }
        return;
    }
    fread(seat_map, sizeof(Seat), 5 * 4 * 4, file);
    fclose(file);
}

// 保存用户计数器到文件（记录已分配的最大用户ID）
void save_user_counter() {
    FILE* file = fopen("Library_UserCounter.bin", "wb");
    if (file == NULL) {
        printf("Error: Failed to save user counter!\n");
        return;
    }
    fwrite(&user_counter, sizeof(int), 1, file);
    fclose(file);
}

// 加载用户计数器（从文件读取已分配的最大用户ID）
void load_user_counter() {
    FILE* file = fopen("Library_UserCounter.bin", "rb");
    if (file == NULL) {
        user_counter = 1;  // 首次运行从1开始分配ID
        return;
    }
    fread(&user_counter, sizeof(int), 1, file);
    fclose(file);
}

// 判断两个时间段是否冲突（重叠即冲突）
int is_time_conflict(Time t1, Time t2) {
    // 冲突条件：t1开始早于t2结束，且t1结束晚于t2开始
    return (t1.start_hour < t2.end_hour) && (t1.end_hour > t2.start_hour);
}

// 验证时间段合法性（开始小时<结束小时，且在0-23范围内）
int is_time_valid(Time time) {
    return (time.start_hour >= 0 && time.start_hour < 23) 
        && (time.end_hour > time.start_hour && time.end_hour <= 23);
}

// 验证预约时长合法性（最长4小时，避免长期占用）
int is_duration_valid(Time time) {
    return (time.end_hour - time.start_hour) <= 4;
}

// 自动清理：过期预约+临时离开超时座位
void auto_clean_expired() {
    time_t now = time(NULL);
    struct tm* current_tm = localtime(&now);
    int cleaned_count = 0;

    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                // 1. 清理已过期的正常预约（当前时间超过预约结束小时）
                if (seat->is_occupied > 0) {
                    if (current_tm->tm_hour >= seat->reservation.time_slot.end_hour) {
                        seat->is_occupied = 0;
                        memset(&seat->reservation, 0, sizeof(RequestInfo));
                        cleaned_count++;
                    }
                }
                // 2. 清理临时离开超时的座位（离开时间超过设定阈值）
                else if (seat->is_occupied < 0 && seat->reservation.leave_time != 0) {
                    if (difftime(now, seat->reservation.leave_time) > LEAVE_TIMEOUT) {
                        seat->is_occupied = 0;
                        memset(&seat->reservation, 0, sizeof(RequestInfo));
                        cleaned_count++;
                    }
                }
            }
        }
    }

    if (cleaned_count > 0) {
        save_seat_data();
        printf("Auto-clean completed: %d expired seats released.\n", cleaned_count);
    } else {
        printf("No expired seats to clean.\n");
    }
}

// 检查座位是否可预约（无冲突+空闲）
int can_reserve_seat(int floor, int row, int col, Time new_slot) {
    Seat* seat = &seat_map[floor - 1][row - 1][col - 1];
    // 座位空闲：直接可预约
    if (seat->is_occupied == 0) {
        return 1;
    }
    // 座位被占用/临时离开：检查时间冲突
    return !is_time_conflict(new_slot, seat->reservation.time_slot);
}

// 预约座位（普通用户功能）
void reserve_seat(int user_id) {
    int floor, row, col;
    Time new_slot;
    char time_str[20];

    // 输入座位位置（楼层1-5，行列1-4）
    printf("Enter floor (1-5): ");
    if (scanf("%d", &floor) != 1 || floor < 1 || floor > 5) {
        printf("Invalid floor! Must be 1-5.\n");
        clear_input_buffer();
        return;
    }

    printf("Enter seat (row column, e.g., 1 2): ");
    if (scanf("%d %d", &row, &col) != 2 || row < 1 || row > 4 || col < 1 || col < 1 || col > 4) {
        printf("Invalid seat! Row/column must be 1-4.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // 输入预约时间段
    printf("Enter start hour (0-22): ");
    if (scanf("%d", &new_slot.start_hour) != 1 || new_slot.start_hour < 0 || new_slot.start_hour > 22) {
        printf("Invalid start hour! Must be 0-22.\n");
        clear_input_buffer();
        return;
    }

    printf("Enter end hour (%d-23): ", new_slot.start_hour + 1);
    if (scanf("%d", &new_slot.end_hour) != 1 || new_slot.end_hour <= new_slot.start_hour || new_slot.end_hour > 23) {
        printf("Invalid end hour! Must be > start hour and <=23.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // 验证时间段合法性
    if (!is_duration_valid(new_slot)) {
        printf("Reservation failed! Max duration is 4 hours.\n");
        return;
    }

    // 检查是否可预约
    if (!can_reserve_seat(floor, row, col, new_slot)) {
        Seat* seat = &seat_map[floor - 1][row - 1][col - 1];
        format_time(seat->reservation.time_slot, time_str);
        printf("Reservation failed! Time conflict with existing reservation (%s).\n", time_str);
        return;
    }

    // 执行预约：更新座位信息
    Seat* seat = &seat_map[floor - 1][row - 1][col - 1];
    seat->is_occupied = user_id;
    seat->reservation.user_id = user_id;
    seat->reservation.time_slot = new_slot;
    seat->reservation.reserve_time = time(NULL);
    seat->reservation.leave_time = 0;  // 初始无离开记录

    // 保存数据并提示
    save_seat_data();
    format_time(new_slot, time_str);
    printf("Reservation successful!\n");
    printf("Seat: Floor %d, Row %d, Col %d | Time: %s\n", floor, row, col, time_str);
}

// 查看指定楼层座位状态（用户/管理员通用）
void view_floor_status() {
    int floor;
    printf("Enter floor to view (1-5): ");
    if (scanf("%d", &floor) != 1 || floor < 1 || floor > 5) {
        printf("Invalid floor! Must be 1-5.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // 打印楼层座位表（0=空闲，正整数=用户ID，负整数=临时离开）
    printf("\n=== Floor %d Seat Status ===\n", floor);
    printf("Row/Col | 1    2    3    4\n");
    printf("---------------------------\n");
    for (int r = 0; r < 4; r++) {
        printf("Row %d   | ", r + 1);
        for (int c = 0; c < 4; c++) {
            int status = seat_map[floor - 1][r][c].is_occupied;
            if (status == 0) {
                printf("Free ");
            } else if (status > 0) {
                printf("U%d   ", status);  // U=正常占用
            } else {
                printf("L%d   ", -status); // L=临时离开
            }
        }
        printf("\n");
    }
    printf("===========================\n");
}

// 查看所有预约（管理员专属）
void view_all_reservations() {
    char time_str[20];
    int has_reservation = 0;

    printf("\n=== All Current Reservations ===\n");
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                if (seat->is_occupied != 0) {
                    format_time(seat->reservation.time_slot, time_str);
                    const char* status = (seat->is_occupied > 0) ? "Occupied" : "Temporarily Away";
                    printf("Floor %d, Seat(%d,%d) | User %d | Time: %s | Status: %s\n",
                           f + 1, r + 1, c + 1, abs(seat->is_occupied), time_str, status);
                    has_reservation = 1;
                }
            }
        }
    }

    if (!has_reservation) {
        printf("No active reservations.\n");
    }
    printf("===============================\n");
}

// 临时离开登记（管理员协助用户操作）
void user_leave(int admin_flag) {
    if (!admin_flag) {
        printf("Permission denied! Only admin can handle leave requests.\n");
        return;
    }

    int user_id;
    printf("Enter user ID to register leave: ");
    if (scanf("%d", &user_id) != 1 || user_id <= 0) {
        printf("Invalid user ID! Must be positive integer.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // 查找用户占用的座位并标记为临时离开
    int updated = 0;
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                if (seat->is_occupied == user_id) {
                    seat->is_occupied = -user_id;  // 负号标记临时离开
                    seat->reservation.leave_time = time(NULL);  // 记录离开时间
                    updated = 1;
                    printf("Leave registered! User %d (Floor %d, Seat(%d,%d)) is temporarily away.\n",
                           user_id, f + 1, r + 1, c + 1);
                }
            }
        }
    }

    if (updated) {
        save_seat_data();
    } else {
        printf("User %d has no occupied seats.\n", user_id);
    }
}

// 临时离开返回登记（管理员协助用户操作）
void user_return(int admin_flag) {
    if (!admin_flag) {
        printf("Permission denied! Only admin can handle return requests.\n");
        return;
    }

    int user_id;
    printf("Enter user ID to register return: ");
    if (scanf("%d", &user_id) != 1 || user_id <= 0) {
        printf("Invalid user ID! Must be positive integer.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // 查找用户临时离开的座位并恢复正常占用
    int updated = 0;
    for (int f = 0; f < 5; f++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                Seat* seat = &seat_map[f][r][c];
                if (seat->is_occupied == -user_id) {
                    seat->is_occupied = user_id;  // 恢复为正用户ID
                    seat->reservation.leave_time = 0;  // 清空离开时间
                    updated = 1;
                    printf("Return registered! User %d (Floor %d, Seat(%d,%d)) is back.\n",
                           user_id, f + 1, r + 1, c + 1);
                }
            }
        }
    }

    if (updated) {
        save_seat_data();
    } else {
        printf("User %d has no temporary leave records.\n", user_id);
    }
}

// 强制清理指定座位（管理员专属）
void force_clear_seat(int admin_flag) {
    if (!admin_flag) {
        printf("Permission denied! Only admin can force clear seats.\n");
        return;
    }

    int floor, row, col;
    printf("Enter seat to clear (floor row column, e.g., 1 1 1): ");
    if (scanf("%d %d %d", &floor, &row, &col) != 3 
        || floor < 1 || floor > 5 || row < 1 || row > 4 || col < 1 || col > 4) {
        printf("Invalid seat! Floor(1-5), Row/Col(1-4).\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    Seat* seat = &seat_map[floor - 1][row - 1][col - 1];
    if (seat->is_occupied == 0) {
        printf("Seat is already free!\n");
        return;
    }

    // 确认清理操作
    char confirm[10];
    printf("Confirm force clear seat (Floor %d, Seat(%d,%d))? (yes/no): ", floor, row, col);
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';

    if (strcmp(confirm, "yes") != 0) {
        printf("Force clear canceled.\n");
        return;
    }

    // 执行清理
    seat->is_occupied = 0;
    memset(&seat->reservation, 0, sizeof(RequestInfo));
    save_seat_data();
    printf("Force clear successful! Seat is now free.\n");
}

// 管理员登录（密码验证）
int admin_login() {
    char password[20];
    printf("Enter admin password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    if (strcmp(password, ADMIN_PASSWORD) == 0) {
        printf("Admin login successful!\n");
        return 1;  // 1=管理员身份
    } else {
        printf("Incorrect password! Admin login failed.\n");
        return 0;  // 0=非管理员
    }
}

// 用户登录（分配/验证用户ID）
int user_login() {
    char choice[10];
    printf("Do you have a user ID? (yes/no): ");
    fgets(choice, sizeof(choice), stdin);
    choice[strcspn(choice, "\n")] = '\0';

    // 已有用户ID：直接验证（检查是否有活跃预约）
    if (strcmp(choice, "yes") == 0) {
        int user_id;
        printf("Enter your user ID: ");
        if (scanf("%d", &user_id) != 1 || user_id <= 0 || user_id >= user_counter) {
            printf("Invalid user ID! Please register first.\n");
            clear_input_buffer();
            return -1;  // -1=登录失败
        }
        clear_input_buffer();
        printf("User %d login successful!\n", user_id);
        return user_id;  // 返回用户ID
    }

    // 无用户ID：分配新ID
    else if (strcmp(choice, "no") == 0) {
        int new_id = user_counter++;
        save_user_counter();
        printf("Registration successful! Your user ID is: %d\n", new_id);
        return new_id;  // 返回新分配的用户ID
    }

    else {
        printf("Invalid choice! Please enter 'yes' or 'no'.\n");
        return -1;  // -1=登录失败
    }
}

// 主菜单（根据身份显示不同功能）
void main_menu(int is_admin, int user_id) {
    char command[20];
    while (1) {
        // 普通用户菜单
        if (!is_admin) {
            printf("\n===== Library Seat Reservation System (User %d) =====\n", user_id);
            printf("1. View floor status\n");
            printf("2. Reserve a seat\n");
            printf("3. Re-login\n");
            printf("4. Quit\n");
            printf("Enter command (1-4): ");
        }
        // 管理员菜单
        else {
            printf("\n===== Library Seat Reservation System (Admin) =====\n");
            printf("1. View floor status\n");
            printf("2. View all reservations\n");
            printf("3. Register user leave\n");
            printf("4. Register user return\n");
            printf("5. Auto-clean expired seats\n");
            printf("6. Force clear a seat\n");
            printf("7. Re-login\n");
            printf("8. Quit\n");
            printf("Enter command (1-8): ");
        }

        // 读取命令
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        // 普通用户功能处理
        if (!is_admin) {
            if (strcmp(command, "1") == 0) {
                view_floor_status();
            } else if (strcmp(command, "2") == 0) {
                reserve_seat(user_id);
            } else if (strcmp(command, "3") == 0) {
                printf("Re-logging in...\n");
                break;  // 退出当前循环，返回重新登录
            } else if (strcmp(command, "4") == 0) {
                printf("Thank you for using! Goodbye.\n");
                exit(0);  // 退出程序
            } else {
                printf("Invalid command! Please enter 1-4.\n");
            }
        }
        // 管理员功能处理
        else {
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
                break;  // 退出当前循环，返回重新登录
            } else if (strcmp(command, "8") == 0) {
                printf("Thank you for using! Goodbye.\n");
                exit(0);  // 退出程序
            } else {
                printf("Invalid command! Please enter 1-8.\n");
            }
        }
    }
}

// 程序入口
int main() {
    // 初始化：加载历史数据
    load_seat_data();
    load_user_counter();

    printf("Welcome to Library Seat Reservation System!\n");

    // 登录循环（支持重新登录）
    while (1) {
        char user_type[10];
        printf("\nLogin as? (user/admin): ");
        fgets(user_type, sizeof(user_type), stdin);
        user_type[strcspn(user_type, "\n")] = '\0';

        // 普通用户登录
        if (strcmp(user_type, "user") == 0) {
            int user_id = user_login();
            if (user_id != -1) {
                main_menu(0, user_id);  // 0=非管理员
            }
        }
        // 管理员登录
        else if (strcmp(user_type, "admin") == 0) {
            int is_admin = admin_login();
            if (is_admin) {
                main_menu(1, 0);  // 1=管理员，user_id填0（无意义）
            }
        }
        // 无效选择
        else {
            printf("Invalid user type! Please enter 'user' or 'admin'.\n");
        }
    }

    return 0;
}
