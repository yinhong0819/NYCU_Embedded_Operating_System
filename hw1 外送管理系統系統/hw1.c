#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> //O_WRONLY
#include <errno.h>
#define LED_DEVICE "/dev/led_dev" 
#define SEG_DEVICE "/dev/seg_dev" 

// Define store and food data structures
struct MenuItem {
    char *name;
    int price;
};

struct Restaurant {
    char *name;
    int distance;
    struct MenuItem menu[2];
};

struct Restaurant restaurants[3] = {
    {"Dessert shop", 3, {{"cookie", 60}, {"cake", 80}}},
    {"Beverage shop", 5, {{"tea", 40}, {"boba", 70}}},
    {"Diner", 8, {{"fried rice", 120}, {"egg-drop soup", 50}}}
};

// main menu
void showMainMenu() {
    printf("1. Shop List\n");
    printf("2. Order\n");
}

// Select store
void showShopList() {
    printf("Shop List:\n");
    for (int i = 0; i < 3; i++) {
        printf("%d. %s: %dkm\n", i + 1, restaurants[i].name, restaurants[i].distance);
    }
}

// Select meal and number of portions
void showOrderMenu(int restaurantIdx) {
    printf("Menu for %s\n", restaurants[restaurantIdx].name);
    for (int i = 0; i < 2; i++) {
        printf("%d. %s: $%d\n", i + 1, restaurants[restaurantIdx].menu[i].name, restaurants[restaurantIdx].menu[i].price);
    }
    printf("3. Confirm\n");
    printf("4. Cancel\n");
}

int main() {
    int fd1, fd2;
    int choice;

    fd1 = open(SEG_DEVICE, O_WRONLY);   
    fd2 = open(LED_DEVICE, O_WRONLY);

    if (fd1 < 0) {
        perror("Failed to open the seg device");
        return errno;
    }
    if (fd2 < 0) {
        perror("Failed to open the led device");
        return errno;
    }

    while (1) {
        showMainMenu();
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            showShopList();
            printf("Press any key to go back to the main menu.\n");
            getchar(); // get 1
            getchar(); // Used to clear the input buffer
        } 
        else if (choice == 2) {
            int restaurantChoice = 0;
            int quantity[2] = {0};

            showShopList();
            printf("Choose a restaurant: ");
            scanf("%d", &restaurantChoice);

            if (restaurantChoice >= 1 && restaurantChoice <= 3) {
                int orderChoice;

                while (1) {
                    showOrderMenu(restaurantChoice - 1);
                    printf("Enter choice: ");
                    scanf("%d", &orderChoice);

                    if (orderChoice >= 1 && orderChoice <= 2) {
                        printf("How many: ");
                        scanf("%d", &quantity[orderChoice - 1]);
                    } 
                    else if (orderChoice == 3) {  //confirm
                        int total = 0;
                        char str[20];
                        for (int i = 0; i < 2; i++) {
                            total += quantity[i] * restaurants[restaurantChoice - 1].menu[i].price;
                        }
                        printf("Total amount: %d\n", total);

                        // Display amount(7-segment display)
                        int length = snprintf(str, sizeof(str), "%d", total);  //Convert integer to string
                        for (int i = 0; i < length; i++) {
                            // printf("%c ", str[i]);
                            if(write(fd1,&str[i],1) < 0){
                                perror("error seg write");
                                exit(EXIT_FAILURE);
                            }
                            fflush(stdout);
                            usleep(500000); //The unit is milliseconds
                        }
                        printf("Please wait for a few minutes...\n");
                        // close(fd1);

                        // Show distance (LED)
                        int distance = restaurants[restaurantChoice - 1].distance;
                        for (int i = distance ; i >= 0; i--) {
                            if (write(fd2, &i, sizeof(i)) < 0) {
                                perror("Error LED write");
                                exit(EXIT_FAILURE);
                            }
                            fflush(stdout);
                            sleep(1);  // wait 1 second
                        }
                        printf("please pick up your meal\n");
                        // close(fd2);

                        printf("Press any key to go back to the main menu.\n");
                        getchar(); 
                        getchar(); 

                        break;
                    } 
                    else if (orderChoice == 4) {
                        break;
                    }
                }
            } else {
                printf("Invalid choice.\n");
            }
        }
    }

    return 0;
}
