#include <stdio.h>
#include <string.h>

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int login() {
    char username[100];
    int isAdmin = 0;

    while (1) {
        printf("Please enter the username (or 'Quit' to exit): ");
        gets(username);

        if (strcmp(username, "Quit") == 0) {
            return -1; // 表示退出程序
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

    // 登录后操作
    char command[100];
    while (1) {
        printf("Enter OK to continue, Exit to re-login, or Quit to exit: ");
        gets(command);

        if (strcmp(command, "OK") == 0) {
            return isAdmin;
        }
        else if (strcmp(command, "Exit") == 0) {
            return login(); // 重新登录
        }
        else if (strcmp(command, "Quit") == 0) {
            return -1; // 退出程序
        }
        else {
            printf("Invalid command.\n");
        }
    }
}

void displayFloor(int map[5][4][4], int floor) {
    printf("\nFloor %d:\n", floor);
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            printf("%d ", map[floor-1][row][col]);
        }
        printf("\n");
    }
}

void displayAllReservations(int map[5][4][4]) {
    printf("\nAll reservations:\n");
    for (int floor = 0; floor < 5; floor++) {
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                if (map[floor][row][col] != 0) {
                    printf("Floor %d, Seat %d-%d\n", floor+1, row+1, col+1);
                }
            }
        }
    }
}

int main() {
    int map[5][4][4] = {0}; // 初始化所有座位为空
    int isAdmin = login();
    
    if (isAdmin == -1) {
        printf("Program exited.\n");
        return 0;
    }

    char command[100];
    
    while (1) {
        if (isAdmin == 0) { // 普通用户
            printf("\nOptions: View floor, Reserve seat, Continue, Quit\n");
            printf("Enter command: ");
            gets(command);

            if (strcmp(command, "View floor") == 0) {
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();
                
                if (floor >= 1 && floor <= 5) {
                    displayFloor(map, floor);
                } else {
                    printf("Invalid floor number.\n");
                }
            }
            else if (strcmp(command, "Reserve seat") == 0) {
                int floor, row, col;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                printf("Enter seat (row column, e.g., 1 2): ");
                scanf("%d %d", &row, &col);
                clearInputBuffer();
                
                if (floor >= 1 && floor <= 5 && row >= 1 && row <= 4 && col >= 1 && col <= 4) {
                    if (map[floor-1][row-1][col-1] == 0) {
                        map[floor-1][row-1][col-1] = 2; // 2表示已预约
                        printf("Reservation successful.\n");
                        displayFloor(map, floor);
                    } else {
                        printf("Seat already occupied.\n");
                    }
                } else {
                    printf("Invalid input.\n");
                }
            }
            else if (strcmp(command, "Quit") == 0) {
                break;
            }
            else if (strcmp(command, "Continue") == 0) {
                continue;
            }
            else {
                printf("Unknown command.\n");
            }
        }
        else { // 管理员
            printf("\nOptions: View floor, View reservations, Continue, Quit\n");
            printf("Enter command: ");
            gets(command);

            if (strcmp(command, "View floor") == 0) {
                int floor;
                printf("Enter floor number (1-5): ");
                scanf("%d", &floor);
                clearInputBuffer();
                
                if (floor >= 1 && floor <= 5) {
                    printf("\nFloor %d (1=occupied, 0=available):\n", floor);
                    for (int row = 0; row < 4; row++) {
                        for (int col = 0; col < 4; col++) {
                            printf("%d ", map[floor-1][row][col] != 0 ? 1 : 0);
                        }
                        printf("\n");
                    }
                } else {
                    printf("Invalid floor number.\n");
                }
            }
            else if (strcmp(command, "View reservations") == 0) {
                displayAllReservations(map);
            }
            else if (strcmp(command, "Quit") == 0) {
                break;
            }
            else if (strcmp(command, "Continue") == 0) {
                continue;
            }
            else {
                printf("Unknown command.\n");
            }
        }
    }

    printf("Thank you for using the reservation system.\n");
    return 0;
}