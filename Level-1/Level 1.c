#include <stdio.h>
#include <string.h>
#include <windows.h>

// 获取程序自身完整路径
void getProgramPath(char* buffer, int bufferSize) {
    GetModuleFileName(NULL, buffer, bufferSize);
}

// Windows专用的文件锁函数
HANDLE lock_file(const char *filename) {
    HANDLE hFile = CreateFile(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        0,  // 不共享，独占访问
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    
    OVERLAPPED ov = {0};
    BOOL result = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ov);
    if (!result) {
        CloseHandle(hFile);
        return NULL;
    }
    
    return hFile;
}

int unlock_file(HANDLE lock_handle) {
    if (lock_handle == NULL || lock_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    OVERLAPPED ov = {0};
    BOOL result = UnlockFileEx(lock_handle, 0, 1, 0, &ov);
    CloseHandle(lock_handle);
    return result;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void safeReadData(int map[5][4][4]) {
    HANDLE lock_handle = lock_file("Reservations.lock");
    if (lock_handle == NULL) {
        printf("Warning: Failed to acquire data file lock, using empty data\n");
        memset(map, 0, sizeof(int) * 5 * 4 * 4);
        return;
    }
    
    FILE *file = fopen("Reservations.bin", "rb");
    if (file) {
        fread(map, sizeof(int), 5 * 4 * 4, file);
        fclose(file);
    } else {
        memset(map, 0, sizeof(int) * 5 * 4 * 4);
    }
    
    unlock_file(lock_handle);
}

void safeWriteData(int map[5][4][4]) {
    HANDLE lock_handle = lock_file("Reservations.lock");
    if (lock_handle == NULL) {
        printf("Error: Failed to acquire data file lock, save failed\n");
        return;
    }
    
    FILE *file = fopen("Reservations.bin", "wb");
    if (file) {
        fwrite(map, sizeof(int), 5 * 4 * 4, file);
        fclose(file);
        printf("Data saved successfully\n");
    } else {
        printf("Error: Failed to open data file for writing\n");
    }
    
    unlock_file(lock_handle);
}

void readUser(int* userCounter) {
    HANDLE lock_handle = lock_file("User.lock");
    if (lock_handle == NULL) {
        printf("Warning: Failed to acquire user file lock, using default ID\n");
        *userCounter = 1;
        return;
    }
    
    FILE *file = fopen("User.bin", "rb");
    if (file) {
        fread(userCounter, sizeof(int), 1, file);
        fclose(file);
    } else {
        *userCounter = 1;
    }
    
    unlock_file(lock_handle);
}

void writeUser(int userCounter) {
    HANDLE lock_handle = lock_file("User.lock");
    if (lock_handle == NULL) {
        printf("Error: Failed to acquire user file lock, save failed\n");
        return;
    }
    
    FILE *file = fopen("User.bin", "wb");
    if (file) {
        fwrite(&userCounter, sizeof(int), 1, file);
        fclose(file);
    }
    
    unlock_file(lock_handle);
}

int getNextUserId() {
    HANDLE lock_handle = lock_file("User.lock");
    if (lock_handle == NULL) {
        printf("Error: Failed to acquire user ID lock\n");
        return -1;
    }
    
    int userCounter = 1;
    FILE *file = fopen("User.bin", "rb");
    if (file) {
        fread(&userCounter, sizeof(int), 1, file);
        fclose(file);
    }
    
    int newUserId = userCounter++;
    
    file = fopen("User.bin", "wb");
    if (file) {
        fwrite(&userCounter, sizeof(int), 1, file);
        fclose(file);
    }
    
    unlock_file(lock_handle);
    return newUserId;
}

void clear(int map[5][4][4], int *userCounter) {
    // 获取数据文件锁
    HANDLE data_lock = lock_file("Reservations.lock");
    if (data_lock == NULL) {
        printf("Error: Failed to acquire data file lock\n");
        return;
    }
    
    // 获取用户文件锁
    HANDLE user_lock = lock_file("User.lock");
    if (user_lock == NULL) {
        printf("Error: Failed to acquire user file lock\n");
        unlock_file(data_lock);
        return;
    }
    
    memset(map, 0, sizeof(int) * 5 * 4 * 4);
    *userCounter = 1;
    
    FILE *file = fopen("Reservations.bin", "wb");
    if (file) {
        fwrite(map, sizeof(int), 5 * 4 * 4, file);
        fclose(file);
    }
    
    FILE *file1 = fopen("User.bin", "wb");
    if (file1) {
        fwrite(userCounter, sizeof(int), 1, file1);
        fclose(file1);
    }
    
    unlock_file(data_lock);
    unlock_file(user_lock);
    printf("Clear successful! All data has been reset\n");
}

int Enter(int isAdmin, int* userCounter) {
    char username[100];
    readUser(userCounter);

    while (1) {
        printf("Please enter username (or enter 'Quit' to exit): ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';

        if (strcmp(username, "Quit") == 0) {
            isAdmin = -1;
            return isAdmin;
        }
        else if (strcmp(username, "A") == 0) {
            printf("User login successful\n");
            isAdmin = 0;
            break;
        }
        else if (strcmp(username, "admin") == 0) {
            printf("Admin login successful\n");
            isAdmin = 1;
            break;
        }
        else {
            printf("Invalid username, please try again\n");
        }
    }

    char command[100];
    while (1) {
        printf("Enter 'OK' to continue, 'Exit' to re-login, or 'Quit' to exit: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "OK") == 0) {
            return isAdmin;
        }
        else if (strcmp(command, "Exit") == 0) {
            return Enter(isAdmin, userCounter);
        }
        else if (strcmp(command, "Quit") == 0) {
            isAdmin = -1;
            return isAdmin;
        }
        else {
            printf("Invalid command\n");
        }
    }
}

void displayFloor(int map[5][4][4], int floor) {
    printf("\nFloor %d seat status (0=Available, Number=User ID):\n", floor);
    for (int row = 0; row < 4; row++) {
        for (int c = 0; c < 4; c++) {
            printf("%d ", map[floor-1][row][c]);
        }
        printf("\n");
    }
}

void displayAllReservations(int map[5][4][4]) {
    printf("\nAll reservation information:\n");
    int found = 0;
    for (int floor = 0; floor < 5; floor++) {
        for (int row = 0; row < 4; row++) {
            for (int c = 0; c < 4; c++) {
                if (map[floor][row][c] != 0) {
                    printf("Floor %d, Seat %d-%d (User %d)\n", 
                           floor+1, row+1, c+1, map[floor][row][c]);
                    found = 1;
                }
            }
        }
    }
    if (!found) {
        printf("No reservation records available\n");
    }
}

// Windows专用的多开启动函数
void launchMultipleInstances(int count) {
    printf("Launching %d instances...\n", count);
    
    char programPath[MAX_PATH];
    getProgramPath(programPath, MAX_PATH);
    printf("Program path: %s\n", programPath);
    
    for (int i = 0; i < count; i++) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // 构建命令行，确保使用完整路径
        char commandLine[MAX_PATH + 10];
        sprintf(commandLine, "\"%s\"", programPath);
        
        // 启动新实例
        if (CreateProcess(
            NULL,           // 不指定可执行文件路径
            commandLine,    // 使用完整路径的命令行
            NULL,           // 进程句柄不可继承
            NULL,           // 线程句柄不可继承
            FALSE,          // 不继承句柄
            CREATE_NEW_CONSOLE, // 创建新控制台窗口
            NULL,           // 使用父进程环境块
            NULL,           // 使用父进程目录
            &si,            // 启动信息
            &pi             // 进程信息
        )) {
            printf("Instance %d launched successfully (PID: %lu)\n", i+1, pi.dwProcessId);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            DWORD error = GetLastError();
            printf("Instance %d failed to launch, error code: %lu\n", i+1, error);
            
            // 尝试使用当前目录的相对路径
            printf("Trying to launch with relative path...\n");
            if (CreateProcess(
                programPath,    // 直接指定可执行文件路径
                NULL,           // 无命令行参数
                NULL, 
                NULL, 
                FALSE, 
                CREATE_NEW_CONSOLE, 
                NULL, 
                NULL, 
                &si, 
                &pi
            )) {
                printf("Instance %d launched successfully! (PID: %lu)\n", i+1, pi.dwProcessId);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } else {
                printf("Instance %d still failed\n", i+1);
            }
        }
        
        Sleep(1000); // 间隔1秒启动下一个实例
    }
}

int main() {
    int map[5][4][4] = {0};
    int userCounter = 1;
    int isAdmin = 0;
    
    // 显示当前进程信息
    printf("=== Seat Reservation System (Windows Multi-instance Version) ===\n");
    printf("Process ID: %lu\n", GetCurrentProcessId());
    printf("Startup Time: %lu\n", GetTickCount());
    
    // 检查是否通过多开启动器启动
    if (__argc > 1) {
        if (strcmp(__argv[1], "multi") == 0) {
            int instances = 3;
            if (__argc > 2) {
                instances = atoi(__argv[2]);
            }
            launchMultipleInstances(instances);
            return 0;
        }
    }
    
    // 登录
    isAdmin = Enter(isAdmin, &userCounter);
    if (isAdmin == -1) {
        printf("Program exited\n");
        return 0;
    }

    // 读取数据
    safeReadData(map);
    
    // 分配用户ID
    int currentUserId;
    if (isAdmin == 1) {
        currentUserId = 0;  // 管理员ID为0
        printf("Current identity: Admin\n");
    } else {
        currentUserId = getNextUserId();
        if (currentUserId == -1) {
            printf("Error: Failed to assign user ID\n");
            return 1;
        }
        printf("Current identity: Regular User (ID: %d)\n", currentUserId);
        writeUser(userCounter);  // 更新用户计数器
    }

    char command[100];
    
    while (1) {
        if (isAdmin == 0) { // 用户模式
            printf("\nAvailable commands: View floor, Reserve seat, Re-login, Quit, MultiTest\n");
            printf("Please enter command: ");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';
            
            if (strcmp(command, "View floor") == 0) {
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();
                
                if (floor >= 1 && floor <= 5) {
                    safeReadData(map);  // 读取最新数据
                    displayFloor(map, floor);
                } else {
                    printf("Invalid floor number\n");
                }
            }
            else if (strcmp(command, "Reserve seat") == 0) {
                int floor, row, c;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                printf("Enter seat (row column, e.g.: 1 2): ");
                scanf("%d %d", &row, &c);
                clearInputBuffer();
                
                if (floor >= 1 && floor <= 5 && row >= 1 && row <= 4 && c >= 1 && c <= 4) {
                    safeReadData(map);  // 读取最新数据
                    if (map[floor-1][row-1][c-1] == 0) {
                        map[floor-1][row-1][c-1] = currentUserId;
                        safeWriteData(map);  // 安全保存数据
                        printf("Reservation successful! Your User ID: %d\n", currentUserId);
                        displayFloor(map, floor);
                    } else {
                        printf("Seat is occupied by User %d\n", map[floor-1][row-1][c-1]);
                    }
                } else {
                    printf("Invalid input\n");
                }
            }
            else if (strcmp(command, "Quit") == 0) {
                break;
            }
            else if (strcmp(command, "Re-login") == 0) {
                isAdmin = Enter(isAdmin, &userCounter);
                if (isAdmin == -1) break;
                if (isAdmin == 0) {
                    currentUserId = getNextUserId();
                    if (currentUserId != -1) {
                        writeUser(userCounter);
                    }
                } else {
                    currentUserId = 0;
                }
            }
            else if (strcmp(command, "MultiTest") == 0) {
                printf("Launching multi-instance test...\n");
                launchMultipleInstances(2);
            }
            else {
                printf("Unknown command\n");
            }
        }
        else { // 管理员模式
            printf("\nAvailable commands: View floor, View reservations, Clear, Re-login, Quit, MultiTest\n");
            printf("Please enter command: ");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "View floor") == 0) {
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();
                
                if (floor >= 1 && floor <= 5) {
                    safeReadData(map);
                    displayFloor(map, floor);
                } else {
                    printf("Invalid floor number\n");
                }
            }
            else if (strcmp(command, "View reservations") == 0) {
                safeReadData(map);
                displayAllReservations(map);
            }
            else if (strcmp(command, "Quit") == 0) {
                break;
            }
            else if (strcmp(command, "Re-login") == 0) {
                isAdmin = Enter(isAdmin, &userCounter);
                if (isAdmin == -1) break;
                if (isAdmin == 0) {
                    currentUserId = getNextUserId();
                    if (currentUserId != -1) {
                        writeUser(userCounter);
                    }
                } else {
                    currentUserId = 0;
                }
            }
            else if (strcmp(command, "Clear") == 0) {
                printf("Confirm to clear all data? (Enter 'yes' to confirm): ");
                char confirm[10];
                fgets(confirm, sizeof(confirm), stdin);
                confirm[strcspn(confirm, "\n")] = '\0';
                if (strcmp(confirm, "yes") == 0) {
                    clear(map, &userCounter);
                    safeReadData(map);  // 重新读取清除后的数据
                } else {
                    printf("Clear operation cancelled\n");
                }
            }
            else if (strcmp(command, "MultiTest") == 0) {
                printf("Launching multi-instance test...\n");
                launchMultipleInstances(3);
            }
            else {
                printf("Unknown command\n");
            }
        }
    }

    printf("Thank you for using the Seat Reservation System!\n");
    return 0;
}