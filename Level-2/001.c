map[floor-1][row-1][c-1] = currentUserId; // 使用用户ID标记预约
                        writeData(map);//读入座位号
                        printf("Reservation successful. Your user ID: %d\n", currentUserId);
                        displayFloor(map, floor);