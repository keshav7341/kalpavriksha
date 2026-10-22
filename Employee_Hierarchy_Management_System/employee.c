#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REPORTS 10
#define MAX_EMPLOYEES 100

struct Employee {
    int id;
    char name[50];
    char designation[50];
    struct Employee *reports[MAX_REPORTS];
    struct Employee *reporting_manager;
    int num_reports;
};

// keeps all the top-level employees (the ones with no manager)
// fixed size of 100, numRoots tells how many are actually used
struct Employee *roots[MAX_EMPLOYEES];
int numRoots = 0;

void display_menu() {
    printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
    printf("1. Add Employee\n");
    printf("2. Remove Employee\n");
    printf("3. Display Employee Hierarchy\n");
    printf("4. Display Complete Hierarchy\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

//  recursive helper functions 

// looks for the id in this employee and everyone under them
struct Employee *findInTree(struct Employee *emp, int id) {
    if (emp->id == id) {
        return emp;
    }

    for (int i = 0; i < emp->num_reports; i++) {
        struct Employee *found = findInTree(emp->reports[i], id);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

// goes through each top-level employee's tree one by one, starting from roots[index]
struct Employee *findInRoots(int index, int id) {
    if (index >= numRoots) {
        return NULL;
    }

    struct Employee *found = findInTree(roots[index], id);
    if (found != NULL) {
        return found;
    }
    return findInRoots(index + 1, id);
}

// finds an employee by id anywhere in the company, returns NULL if not found
struct Employee *findEmployee(int id) {
    return findInRoots(0, id);
}

// counts everyone working under this employee (the employee is not counted)
int countUnder(struct Employee *emp) {
    int count = 0;

    for (int i = 0; i < emp->num_reports; i++) {
        count += 1 + countUnder(emp->reports[i]);
    }
    return count;
}

// prints the employee first, then does the same for each person reporting to them
void printTree(struct Employee *emp, int level) {
    printf("%*s%d - %s - %s\n",
           level * 4, "", emp->id, emp->name, emp->designation);

    for (int i = 0; i < emp->num_reports; i++) {
        printTree(emp->reports[i], level + 1);
    }
}

// prints the tree of every top-level employee, starting from roots[index]
void printRoots(int index) {
    if (index >= numRoots) {
        return;
    }

    printTree(roots[index], 0);
    printRoots(index + 1);
}

// counts all employees in the company (top-level ones + everyone under them)
int countRoots(int index) {
    if (index >= numRoots) {
        return 0;
    }

    return 1 + countUnder(roots[index]) + countRoots(index + 1);
}

// frees the employee and everyone under them (children first, then the employee)
void freeTree(struct Employee *emp) {
    for (int i = 0; i < emp->num_reports; i++) {
        freeTree(emp->reports[i]);
    }
    free(emp);
}

// frees every top-level employee's tree, starting from roots[index]
void freeRoots(int index) {
    if (index >= numRoots) {
        return;
    }

    freeTree(roots[index]);
    freeRoots(index + 1);
}

// 1. add employee 
void addEmployee() {
    int id, managerId;
    char name[50], designation[50];

    printf("Enter Employee ID: ");
    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid ID. It must be a positive number.\n");
        while (getchar() != '\n');
        return;
    }

    // id must be unique, so check if someone already has it
    if (findEmployee(id) != NULL) {
        printf("Employee ID %d already exists.\n", id);
        return;
    }

    printf("Enter Name: ");
    scanf(" %49[^\n]", name);

    printf("Enter Designation: ");
    scanf(" %49[^\n]", designation);

    printf("Enter Reporting Manager ID (0 if top-level): ");
    scanf("%d", &managerId);

    struct Employee *manager = NULL;

    if (managerId != 0) {
        // an employee can't report to themselves
        if (managerId == id) {
            printf("An employee cannot report to themselves.\n");
            return;
        }

        manager = findEmployee(managerId);

        if (manager == NULL) {
            printf("Manager ID %d does not exist.\n", managerId);
            return;
        }

        // manager can have only 10 reports, so check there is space left
        if (manager->num_reports >= MAX_REPORTS) {
            printf("Manager %d already has the maximum of %d reports.\n",
                   managerId, MAX_REPORTS);
            return;
        }
    } else if (numRoots >= MAX_EMPLOYEES) {
        // no space left in the roots list for another top-level employee
        printf("Cannot add more top-level employees (limit is %d).\n", MAX_EMPLOYEES);
        return;
    }

    struct Employee *emp = (struct Employee *)malloc(sizeof(struct Employee));

    if (emp == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    emp->id = id;
    strcpy(emp->name, name);
    strcpy(emp->designation, designation);
    emp->num_reports = 0;
    emp->reporting_manager = manager;

    // new employee, so nobody is reporting to them yet
    for (int i = 0; i < MAX_REPORTS; i++) {
        emp->reports[i] = NULL;
    }

    if (manager != NULL) {
        // add the employee to the manager's reports list
        manager->reports[manager->num_reports++] = emp;
    } else {
        // no manager means top-level, so add to the roots list
        roots[numRoots++] = emp;
    }

    // no circular reporting can happen here since the new employee
    // has nobody reporting to them yet
    printf("Employee %d (%s) added successfully.\n", id, name);
}

// 2. remove employee   

void removeEmployee() {
    int id;

    printf("Enter Employee ID to remove: ");

    scanf("%d", &id);

    struct Employee *emp = findEmployee(id);

    if (emp == NULL) {
        printf("Employee %d not found.\n", id);
        return;
    }

    struct Employee *manager = emp->reporting_manager;

    if (manager != NULL) {
        // make sure the manager has enough space for the promoted employees
        if (manager->num_reports - 1 + emp->num_reports > MAX_REPORTS) {
            printf("Cannot remove %d: manager %d would exceed %d reports.\n",
                   id, manager->id, MAX_REPORTS);
            return;
        }

        // take the employee out of the manager's reports list
        int idx = 0;
        while (manager->reports[idx] != emp) {
            idx++;
        }
        for (int i = idx; i < manager->num_reports - 1; i++) {
            manager->reports[i] = manager->reports[i + 1];
        }
        manager->reports[--manager->num_reports] = NULL;

        // give the removed employee's reports to the manager
        for (int i = 0; i < emp->num_reports; i++) {
            struct Employee *child = emp->reports[i];
            child->reporting_manager = manager;
            manager->reports[manager->num_reports++] = child;
        }
    } else {
        // employee is top-level, so their reports take their place in the roots list
        int pos = 0;
        while (roots[pos] != emp) {
            pos++;
        }

        int n = emp->num_reports;
        int newCount = numRoots - 1 + n;

        // the promoted reports must fit in the roots list
        if (newCount > MAX_EMPLOYEES) {
            printf("Cannot remove %d: too many top-level employees (limit is %d).\n",
                   id, MAX_EMPLOYEES);
            return;
        }

        // shift the remaining employees to make room (or to close the gap)
        memmove(&roots[pos + n], &roots[pos + 1],
                (numRoots - pos - 1) * sizeof(struct Employee *));

        for (int i = 0; i < n; i++) {
            emp->reports[i]->reporting_manager = NULL;
            roots[pos + i] = emp->reports[i];
        }
        numRoots = newCount;
    }

    printf("Employee %d (%s) removed successfully.\n", emp->id, emp->name);

    free(emp);
}

//  3. display hierarchy of one employee 
void displayEmployeeHierarchy() {
    int id;

    printf("Enter Employee ID: ");

    scanf("%d", &id);

    struct Employee *emp = findEmployee(id);

    if (emp == NULL) {
        printf("Employee %d not found.\n", id);
        return;
    }

    printf("\n");
    printTree(emp, 0);

    printf("Total Employees Under %d: %d\n", id, countUnder(emp));
}

// 4. display complete hierarchy 
void displayCompleteHierarchy() {
    if (numRoots == 0) {
        printf("No employees in the organization.\n");
        return;
    }

    printf("\n===== ORGANIZATION HIERARCHY =====\n");
    printRoots(0);
    printf("Total Employees: %d\n", countRoots(0));
}

int main() {
    int choice;

    do {
        display_menu();

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                addEmployee();
                break;

            case 2:
                removeEmployee();
                break;

            case 3:
                displayEmployeeHierarchy();
                break;

            case 4:
                displayCompleteHierarchy();
                break;

            case 5:
                printf("Exiting the program.\n");
                freeRoots(0);
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }

    } while (1);

    return 0;
}