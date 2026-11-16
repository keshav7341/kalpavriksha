#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_BLOCKS 1024
#define BLOCK_SIZE 512

char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];

// Structure to represent a file or directory in the virtual file system
typedef struct FileNode
{
    char name[50];
    int size;
    bool isDirectory; // 1 for directory, 0 for file
    int numBlocks;
    int blockIndices[NUM_BLOCKS];
    struct FileNode *parent;
    struct FileNode *child;
    struct FileNode *next;
    struct FileNode *prev;
} FileNode;

FileNode *createFileNode(char *name, bool isDirectory);
FileNode *root;
FileNode *cwd;

// Structure to represent a free block in the virtual disk
typedef struct FreeBlock
{
    int blockIndex;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

FreeBlock *head = NULL;
FreeBlock *tail = NULL;

int totalBlocks = NUM_BLOCKS;
int freeBlockCount = NUM_BLOCKS;

// disk usage in percentage
float diskUsage()
{
    return (float)(totalBlocks - freeBlockCount) / totalBlocks * 100;
}

// create all free blocks at the start
FreeBlock *initializeFreeBlocks()
{
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        FreeBlock *newNode = (FreeBlock *)malloc(sizeof(FreeBlock));
        if (newNode == NULL)
        {
            printf("Memory allocation failed!\n");
            return NULL;
        }
        newNode->blockIndex = i;
        newNode->next = NULL;

        if (head == NULL)
        {
            head = newNode;
            tail = newNode;
            newNode->prev = NULL;
        }
        else
        {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }
    return head;
}

// when a block is freed, add it at the tail of the free list
void addFreeBlock(int idx)
{
    FreeBlock *newNode = (FreeBlock *)malloc(sizeof(FreeBlock));
    newNode->blockIndex = idx;
    newNode->next = NULL;
    newNode->prev = NULL;

    if (tail == NULL)
    { // list was empty
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    freeBlockCount++;
}

FileNode *createFileNode(char *name, bool isDirectory)
{
    FileNode *newNode = (FileNode *)malloc(sizeof(FileNode));
    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    strcpy(newNode->name, name);
    newNode->isDirectory = isDirectory;
    newNode->size = 0;
    newNode->numBlocks = 0;
    newNode->parent = NULL;
    newNode->child = NULL;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

int calculateBlocksNeeded(int size)
{
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

// takes blocks from the free list and copies the data into them
int assignBlocksToFile(FileNode *fileNode, char *data)
{
    int size = strlen(data);
    int blocksNeeded = calculateBlocksNeeded(size);

    if (blocksNeeded > freeBlockCount)
    {
        printf("Not enough free blocks!\n");
        return 0;
    }

    fileNode->size = size;
    fileNode->numBlocks = blocksNeeded;

    for (int i = 0; i < blocksNeeded; i++)
    {
        FreeBlock *temp = head;
        int idx = temp->blockIndex;
        fileNode->blockIndices[i] = idx;

        head = head->next;
        if (head != NULL)
            head->prev = NULL;
        else
            tail = NULL;
        free(temp);
        freeBlockCount--;

        int offset = i * BLOCK_SIZE;
        int chunk = size - offset;
        if (chunk > BLOCK_SIZE)
            chunk = BLOCK_SIZE;
        memcpy(virtualDisk[idx], data + offset, chunk);
    }
    return 1;
}

void readFile(FileNode *fileNode)
{
    if (fileNode->numBlocks == 0)
    {
        printf("empty\n");
        return;
    }

    for (int i = 0; i < fileNode->numBlocks; i++)
    {
        int idx = fileNode->blockIndices[i];
        int offset = i * BLOCK_SIZE;
        int chunk = fileNode->size - offset;
        if (chunk > BLOCK_SIZE)
            chunk = BLOCK_SIZE;
        printf("%.*s", chunk, virtualDisk[idx]);
    }
    printf("\n");
}

// give all blocks of a file back to the free list
void freeFileBlocks(FileNode *fileNode)
{
    for (int i = 0; i < fileNode->numBlocks; i++)
    {
        addFreeBlock(fileNode->blockIndices[i]);
    }
    fileNode->numBlocks = 0;
    fileNode->size = 0;
}

// circular list of children

// insert node at the end of parent's circular list
void insertNode(FileNode *parent, FileNode *node)
{
    node->parent = parent;

    if (parent->child == NULL)
    { // empty list, node points to itself
        parent->child = node;
        node->next = node;
        node->prev = node;
    }
    else
    {
        FileNode *first = parent->child;
        FileNode *last = first->prev;
        last->next = node;
        node->prev = last;
        node->next = first;
        first->prev = node;
    }
}

// remove node from its parent's circular list
void removeNode(FileNode *node)
{
    FileNode *parent = node->parent;

    if (node->next == node)
    { // it was the only node
        parent->child = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        if (parent->child == node)
            parent->child = node->next;
    }
}

// search a name in cwd
FileNode *findInCwd(char *name)
{
    if (cwd->child == NULL)
        return NULL;

    FileNode *cur = cwd->child;
    do
    {
        if (strcmp(cur->name, name) == 0)
            return cur;
        cur = cur->next;
    } while (cur != cwd->child);

    return NULL;
}

// commands
void makeDir(char *name)
{
    if (findInCwd(name) != NULL)
    {
        printf("Name already exists.\n");
        return;
    }
    FileNode *dir = createFileNode(name, true);
    insertNode(cwd, dir);
    printf("Directory '%s' created successfully.\n", name);
}

void createFile(char *name)
{
    if (findInCwd(name) != NULL)
    {
        printf("Name already exists.\n");
        return;
    }
    FileNode *file = createFileNode(name, false);
    insertNode(cwd, file);
    printf("File '%s' created successfully.\n", name);
}

void writeFile(char *name, char *data)
{
    FileNode *f = findInCwd(name);
    if (f == NULL || f->isDirectory)
    {
        printf("File not found.\n");
        return;
    }

    // if file already has data, free the old blocks first
    if (f->numBlocks > 0)
        freeFileBlocks(f);

    if (assignBlocksToFile(f, data))
        printf("Data written successfully (size=%d bytes).\n", f->size);
}

void deleteFile(char *name)
{
    FileNode *f = findInCwd(name);
    if (f == NULL || f->isDirectory)
    {
        printf("File not found.\n");
        return;
    }
    freeFileBlocks(f);
    removeNode(f);
    free(f);
    printf("File deleted successfully.\n");
}

void removeDir(char *name)
{
    FileNode *d = findInCwd(name);
    if (d == NULL || !d->isDirectory)
    {
        printf("Directory not found.\n");
        return;
    }
    if (d->child != NULL)
    {
        printf("Directory is not empty.\n");
        return;
    }
    removeNode(d);
    free(d);
    printf("Directory removed successfully.\n");
}

void listFiles()
{
    if (cwd->child == NULL)
    {
        printf("(empty)\n");
        return;
    }
    FileNode *cur = cwd->child;
    do
    {
        if (cur->isDirectory)
            printf("%s/\n", cur->name);
        else
            printf("%s\n", cur->name);
        cur = cur->next;
    } while (cur != cwd->child);
}

// prints path by going up through parent pointers (recursion)
void printPath(FileNode *n)
{
    if (n == root)
    {
        printf("/");
        return;
    }
    if (n->parent != root)
        printPath(n->parent);
    printf("/%s", n->name);
}

void changeDir(char *name)
{
    if (strcmp(name, "..") == 0)
    {
        if (cwd != root)
            cwd = cwd->parent;
    }
    else
    {
        FileNode *d = findInCwd(name);
        if (d == NULL)
        {
            printf("No such directory.\n");
            return;
        }
        if (!d->isDirectory)
        {
            printf("'%s' is a file, not a directory.\n", name);
            return;
        }
        cwd = d;
    }
    printf("Moved to ");
    printPath(cwd);
    printf("\n");
}

void showDf()
{
    printf("Total Blocks: %d\n", totalBlocks);
    printf("Used Blocks: %d\n", totalBlocks - freeBlockCount);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", diskUsage());
}

// cleanup on exit

void freeTree(FileNode *dir)
{
    if (dir->child == NULL)
        return;

    FileNode *cur = dir->child;
    FileNode *last = cur->prev;

    while (1)
    {
        FileNode *nxt = cur->next;
        bool isLast = (cur == last);

        if (cur->isDirectory)
            freeTree(cur);
        free(cur);

        if (isLast)
            break;
        cur = nxt;
    }
    dir->child = NULL;
}

void freeFreeList()
{
    while (head != NULL)
    {
        FreeBlock *temp = head;
        head = head->next;
        free(temp);
    }
    tail = NULL;
}

int main()
{

    char line[5000];
    char command[20];
    char target[100];
    char text[5000];

    initializeFreeBlocks();
    root = createFileNode("/", 1);
    cwd = root;

    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    while (1)
    {
        printPath(cwd);
        printf(" > ");

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';

        int i = 0; // position in the line
        int j = 0; // position in the word being copied

        // 1. read the first word (the command)
        while (line[i] == ' ')
            i++;
        j = 0;
        while (line[i] != ' ' && line[i] != '\0' && j < 19)
        {
            command[j] = line[i];
            i++;
            j++;
        }
        command[j] = '\0';

        if (j == 0)
            continue; // empty line

        // commands that need no argument
        if (strcmp(command, "exit") == 0)
            break;
        if (strcmp(command, "ls") == 0)
        {
            listFiles();
            continue;
        }
        if (strcmp(command, "pwd") == 0)
        {
            printPath(cwd);
            printf("\n");
            continue;
        }
        if (strcmp(command, "df") == 0)
        {
            showDf();
            continue;
        }

        // 2. read the second word (file or folder name)
        while (line[i] == ' ')
            i++;
        j = 0;
        while (line[i] != ' ' && line[i] != '\0' && j < 99)
        {
            target[j] = line[i];
            i++;
            j++;
        }
        target[j] = '\0';

        if (j == 0)
        {
            printf("Missing argument.\n");
            continue;
        }
        if (strlen(target) > 49)
        {
            printf("Name too long (max 50 characters).\n");
            continue;
        }

        // 3. run the command
        if (strcmp(command, "mkdir") == 0)
            makeDir(target);
            
        else if (strcmp(command, "create") == 0)
            createFile(target);

        else if (strcmp(command, "cd") == 0)
            changeDir(target);

        else if (strcmp(command, "delete") == 0)
            deleteFile(target);

        else if (strcmp(command, "rmdir") == 0)
            removeDir(target);
            
        else if (strcmp(command, "read") == 0)
        {
            FileNode *f = findInCwd(target);
            if (f == NULL || f->isDirectory)
                printf("File not found.\n");
            else
                readFile(f);
        }
        else if (strcmp(command, "write") == 0)
        {
            // the data comes after the name, inside quotes
            while (line[i] == ' ')
                i++;

            char quote = line[i];
            if (quote != '"' && quote != '\'')
            {
                printf("Usage: write <file> \"data\"\n");
                continue;
            }
            i++; // skip the opening quote

            j = 0;
            while (line[i] != quote && line[i] != '\0')
            {
                text[j] = line[i];
                i++;
                j++;
            }
            if (line[i] == '\0')
            {
                printf("Usage: write <file> \"data\"\n");
                continue;
            }
            text[j] = '\0';

            writeFile(target, text);
        }
        else
        {
            printf("Unknown command.\n");
        }
    }

    // release everything before exiting
    freeTree(root);
    free(root);
    freeFreeList();
    printf("Memory released. Exiting program.\n");

    return 0;
}