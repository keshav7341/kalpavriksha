#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct user
{
    int id;
    char name[100];
    int age;
};

int exists(int id)
{
    FILE *f = fopen("users.txt", "r");
    struct user u;
    if (f == NULL)
        return 0;
    while (fscanf(f, "%d,%99[^,],%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == id)
        {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}


void addFun()
{
    struct user u;
    FILE *f;

    printf("Enter id: ");
    scanf("%d", &u.id);
    if (exists(u.id))
    {
        printf("id already present\n");
        return;
    }
    printf("Enter name: ");
    scanf(" %99[^\n]", u.name);
    printf("Enter age: ");
    scanf("%d", &u.age);

    f = fopen("users.txt", "a");
    fprintf(f, "%d,%s,%d\n", u.id, u.name, u.age);
    fclose(f);
    printf("record added\n");
}

void showFun()
{
    FILE *f = fopen("users.txt", "r");
    struct user u;
    int n = 0;

    if (f == NULL)
    {
        printf("file not found\n");
        return;
    }
    printf("\nID\tName\t\tAge\n");
    while (fscanf(f, "%d,%99[^,],%d\n", &u.id, u.name, &u.age) == 3)
    {
        printf("%d\t%-15s\t%d\n", u.id, u.name, u.age);
        n++;
    }
    if (n == 0)
        printf("no records\n");
    fclose(f);
}

void updateFun()
{
    FILE *f, *t;
    struct user u;
    int id, found = 0;

    printf("Enter id to update: ");
    scanf("%d", &id);

    f = fopen("users.txt", "r");
    t = fopen("temp.txt", "w");
    if (f == NULL || t == NULL)
    {
        printf("file error\n");
        return;
    }
    while (fscanf(f, "%d,%99[^,],%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == id)
        {
            found = 1;
            printf("Enter new name: ");
            scanf(" %99[^\n]", u.name);
            printf("Enter new age: ");
            scanf("%d", &u.age);
        }
        fprintf(t, "%d,%s,%d\n", u.id, u.name, u.age);
    }
    fclose(f);
    fclose(t);
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found)
        printf("record updated\n");
    else
        printf("id not found\n");
}

void deleteFun()
{
    FILE *f, *t;
    struct user u;
    int id, found = 0;

    printf("Enter id to delete: ");
    scanf("%d", &id);

    f = fopen("users.txt", "r");
    t = fopen("temp.txt", "w");
    if (f == NULL || t == NULL)
    {
        printf("file error\n");
        return;
    }
    while (fscanf(f, "%d,%99[^,],%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == id)
            found = 1;
        else
            fprintf(t, "%d,%s,%d\n", u.id, u.name, u.age);
    }
    fclose(f);
    fclose(t);
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found)
        printf("record deleted\n");
    else
        printf("id not found\n");
}

int main()
{
    int ch;

    FILE *f = fopen("users.txt", "a");
    if (f == NULL)
    {
        printf("cannot open file\n");
        exit(1);
    }
    fclose(f);
    do
    {
        printf("\n1. Add user\n2. Show users\n3. Update user\n4. Delete user\n5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &ch);
        switch (ch)
        
        {
        case 1:
            addFun();
            break;
        case 2:
            showFun();
            break;
        case 3:
            updateFun();
            break;
        case 4:
            deleteFun();
            break;
        case 5:
            break;
        default:
            printf("wrong choice\n");
        }
    } while (ch != 5);


    return 0;
}