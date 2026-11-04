// Smart Memory Manager
// Simulates a fixed memory pool and hands it out using First Fit,
// Best Fit or Worst Fit.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum pool size
#define POOL_SIZE 3000
// initial number of blocks in the metadata array
#define INITIAL_CAPACITY 5



#define FIRST_FIT 1
#define BEST_FIT 2
#define WORST_FIT 3

// A block of memory in the pool. The id is 0 when the block is free.
typedef struct
{
    int start;
    int size;
    int id; // 0 when the block is free
    int isFree;
} Block;

// The memory manager keeps track of the pool and the blocks in it.
typedef struct
{
    unsigned char *pool;
    int poolSize;
    Block *blocks;
    int count;
    int capacity;
    int nextId;
    int strategy;
} MemoryManager;

// Throws away whatever is left on the current input line.
// Called after every scanf so bad input like "abc" or "12abc" cannot
// stay in the buffer and break the next prompt.
void clearLine()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// Doubles the blocks array when it is full. Returns 0 if realloc fails.
void makeRoomForBlock(MemoryManager *m)
{
    Block *tmp;

    if (m->count < m->capacity)
        return;

    tmp = (Block *)realloc(m->blocks, m->capacity * 2 * sizeof(Block));
    if (tmp == NULL) {
        printf("Could not grow the metadata array. Exiting.\n");
        exit(1);
    }

    m->blocks = tmp;
    m->capacity = m->capacity * 2;
}

// Puts a block at position pos and shifts the later ones one place right.
// Call makeRoomForBlock first.
void insertBlockAt(MemoryManager *m, int pos, Block b)
{
    memmove(&m->blocks[pos + 1], &m->blocks[pos],
            (m->count - pos) * sizeof(Block));
    m->blocks[pos] = b;
    m->count++;
}

// Deletes the block at position pos and shifts the later ones one place left.
void removeBlockAt(MemoryManager *m, int pos)
{
    memmove(&m->blocks[pos], &m->blocks[pos + 1],
            (m->count - pos - 1) * sizeof(Block));
    m->count--;
}

// Returns the index of the allocated block with this id, or -1 if none.
int findAllocation(MemoryManager *m, int id)
{
    int i;
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree == 0 && m->blocks[i].id == id)
            return i;
    }
    return -1;
}

int initManager(MemoryManager *m, int poolSize)
{
    m->pool = (unsigned char *)calloc(poolSize, sizeof(unsigned char));
    if (m->pool == NULL)
        return 0;

    m->blocks = (Block *)malloc(INITIAL_CAPACITY * sizeof(Block));
    if (m->blocks == NULL)
    {
        free(m->pool);
        return 0;
    }

    m->poolSize = poolSize;
    m->capacity = INITIAL_CAPACITY;
    m->count = 1;
    m->nextId = 1;
    m->strategy = FIRST_FIT;

    // at the start the whole pool is a single free block
    m->blocks[0].start = 0;
    m->blocks[0].size = poolSize;
    m->blocks[0].id = 0;
    m->blocks[0].isFree = 1;
    return 1;
}

void destroyManager(MemoryManager *m)
{
    free(m->pool);
    free(m->blocks);
}

// First Fit: the first free block that is big enough.
int firstFit(MemoryManager *m, int size)
{
    int i;
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree && m->blocks[i].size >= size)
            return i;
    }
    return -1;
}

// Best Fit: the smallest free block that is still big enough.
int bestFit(MemoryManager *m, int size)
{
    int i, best = -1;
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree && m->blocks[i].size >= size)
        {
            if (best == -1 || m->blocks[i].size < m->blocks[best].size)
                best = i;
        }
    }
    return best;
}

// Worst Fit: the largest free block, as long as it is big enough.
int worstFit(MemoryManager *m, int size)
{
    int i, worst = -1;
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree && m->blocks[i].size >= size)
        {
            if (worst == -1 || m->blocks[i].size > m->blocks[worst].size)
                worst = i;
        }
    }
    return worst;
}

// Calls the search function that matches the current strategy.
int findFreeBlock(MemoryManager *m, int size)
{
    if (m->strategy == BEST_FIT)
        return bestFit(m, size);
    else if (m->strategy == WORST_FIT)
        return worstFit(m, size);
    else
        return firstFit(m, size);
}

// Takes size bytes out of a free block and marks them as used by id.
// Returns the index of the new allocated block, or -1 if no free block fits.
int allocateRegion(MemoryManager *m, int size, int id)
{
    int idx = findFreeBlock(m, size);
    if (idx == -1)
        return -1;

    // if the free block is bigger than needed, the leftover stays free
    if (m->blocks[idx].size > size) {
        Block rest;

        makeRoomForBlock(m);

        rest.start = m->blocks[idx].start + size;
        rest.size = m->blocks[idx].size - size;
        rest.id = 0;
        rest.isFree = 1;

        m->blocks[idx].size = size;
        insertBlockAt(m, idx + 1, rest);
    }

    m->blocks[idx].isFree = 0;
    m->blocks[idx].id = id;
    return idx;
}
// Joins free blocks that sit next to each other into one bigger block.

void joinAdjacentFreeBlocks(MemoryManager *m)
{
    int i = 0;
    while (i < m->count - 1)
    {
        if (m->blocks[i].isFree && m->blocks[i + 1].isFree)
        {
            m->blocks[i].size += m->blocks[i + 1].size;
            removeBlockAt(m, i + 1);
        }
        else
        {
            i++;
        }
    }
}

int totalFreeBytes(MemoryManager *m)
{
    int i, total = 0;
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree)
            total += m->blocks[i].size;
    }
    return total;
}

// Tells the user why an allocation failed.
void explainFailure(MemoryManager *m, int size)
{
    int i, largest = 0;
    int totalFree = totalFreeBytes(m);

    printf("Allocation failed: no contiguous free block is large enough.\n");

    if (totalFree < size)
    {
        printf("Not enough free memory overall (%d bytes free, %d requested).\n",
               totalFree, size);
        return;
    }

    printf("External fragmentation detected: %d bytes requested, "
           "%d bytes free in total, but the free memory is split up:\n",
           size, totalFree);
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree)
        {
            printf("  - free block of %d bytes at offset %d\n",
                   m->blocks[i].size, m->blocks[i].start);
            if (m->blocks[i].size > largest)
                largest = m->blocks[i].size;
        }
    }
    printf("  The largest single block is %d bytes, which is smaller "
           "than the %d bytes requested.\n",
           largest, size);
}

void allocateMemory(MemoryManager *m, int size)
{
    int idx;

    if (size <= 0 || size > m->poolSize)
    {
        printf("Invalid size. It must be between 1 and %d.\n", m->poolSize);
        return;
    }

        idx = allocateRegion(m, size, m->nextId);
    if (idx == -1) {
        explainFailure(m, size);
        return;
    }
    

    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n",
           m->blocks[idx].id, m->blocks[idx].start, m->blocks[idx].size);
    m->nextId++;
}

void releaseMemory(MemoryManager *m, int id)
{
    int idx;

    if (id <= 0)
    {
        printf("Invalid ID. Allocation IDs are positive numbers.\n");
        return;
    }

    idx = findAllocation(m, id);
    if (idx == -1)
    {
        printf("Release failed: no allocated block with ID %d.\n", id);
        return;
    }

    m->blocks[idx].isFree = 1;
    m->blocks[idx].id = 0;
    joinAdjacentFreeBlocks(m);
    printf("Allocation released.\n");
}

// Makes block i smaller. The part that is cut off becomes free.
void shrinkAllocation(MemoryManager *m, int i, int newSize)
{
    int id = m->blocks[i].id;
    int oldSize = m->blocks[i].size;
    int diff = oldSize - newSize;

    if (i + 1 < m->count && m->blocks[i + 1].isFree)
    {
        // the next block is already free, so just make it larger
        m->blocks[i + 1].start -= diff;
        m->blocks[i + 1].size += diff;
        } else {
        Block tail;

        makeRoomForBlock(m);
        tail.start = m->blocks[i].start + newSize;
        tail.size = diff;
        tail.id = 0;
        tail.isFree = 1;
        insertBlockAt(m, i + 1, tail);
    }
    m->blocks[i].size = newSize;
    joinAdjacentFreeBlocks(m);
    printf("Allocation #%d shrunk from %d to %d bytes.\n", id, oldSize, newSize);
}

// Makes block i bigger. It grows in place if the next block is free and
// large enough, otherwise it is moved to a new place.
void growAllocation(MemoryManager *m, int i, int newSize)
{
    int id = m->blocks[i].id;
    int oldSize = m->blocks[i].size;
    int extra = newSize - oldSize;
    int countBefore, newIdx;

    // first try to take the extra bytes from the free block after it
    if (i + 1 < m->count && m->blocks[i + 1].isFree &&
        m->blocks[i + 1].size >= extra)
    {
        m->blocks[i + 1].start += extra;
        m->blocks[i + 1].size -= extra;
        if (m->blocks[i + 1].size == 0)
            removeBlockAt(m, i + 1);
        m->blocks[i].size = newSize;
        printf("Allocation #%d expanded in place from %d to %d bytes.\n",
               id, oldSize, newSize);
        return;
    }

    // otherwise find a new place and copy the data there
    countBefore = m->count;
        newIdx = allocateRegion(m, newSize, id);
    if (newIdx == -1) {
        printf("Resize failed. Allocation #%d is unchanged.\n", id);
        explainFailure(m, newSize);
        return;
    }

    // if a block was inserted before the old one, the old index moved right
    if (newIdx < i)
        i += m->count - countBefore;

    memcpy(m->pool + m->blocks[newIdx].start,
           m->pool + m->blocks[i].start, oldSize);
    m->blocks[i].isFree = 1;
    m->blocks[i].id = 0;
    joinAdjacentFreeBlocks(m);

    printf("Allocation #%d moved and resized from %d to %d bytes.\n",
           id, oldSize, newSize);
}

void resizeAllocation(MemoryManager *m, int id, int newSize)
{
    int i = -1;

    if (id > 0)
        i = findAllocation(m, id);

    if (i == -1)
    {
        printf("Resize failed: no allocated block with ID %d.\n", id);
        return;
    }
    if (newSize <= 0 || newSize > m->poolSize)
    {
        printf("Invalid size. It must be between 1 and %d.\n", m->poolSize);
        return;
    }

    if (newSize == m->blocks[i].size)
        printf("Allocation #%d is already %d bytes. Nothing to do.\n",
               id, newSize);
    else if (newSize < m->blocks[i].size)
        shrinkAllocation(m, i, newSize);
    else
        growAllocation(m, i, newSize);
}

void displayMap(MemoryManager *m)
{
    int i;

    printf("Memory Map (%d bytes):\n", m->poolSize);
    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree)
            printf("[Free: %d bytes, Start: %d]\n",
                   m->blocks[i].size, m->blocks[i].start);
        else
            printf("[Allocated #%d: %d bytes, Start: %d]\n",
                   m->blocks[i].id, m->blocks[i].size, m->blocks[i].start);
    }
}

void displayStats(MemoryManager *m)
{
    int i;
    int allocBytes = 0, freeBytes = 0;
    int allocBlocks = 0, freeBlocks = 0;
    int largestFree = 0;

    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].isFree)
        {
            freeBytes += m->blocks[i].size;
            freeBlocks++;
            if (m->blocks[i].size > largestFree)
                largestFree = m->blocks[i].size;
        }
        else
        {
            allocBytes += m->blocks[i].size;
            allocBlocks++;
        }
    }

    printf("Total memory: %d bytes\n", m->poolSize);
    printf("Allocated: %d bytes (%d blocks)\n", allocBytes, allocBlocks);
    printf("Free: %d bytes\n", freeBytes);
    printf("Free blocks: %d\n", freeBlocks);
    printf("Largest free block: %d bytes\n", largestFree);

    if (freeBlocks > 1)
    {
        printf("Note: free memory is split into %d separate blocks, so a "
               "request larger than %d bytes will fail even though %d bytes "
               "are free.\n",
               freeBlocks, largestFree, freeBytes);
    }
}

// Sanity check run after every menu action: blocks must cover the pool with
// no gaps or overlaps, and two free blocks must never be neighbours.
int mapIsConsistent(MemoryManager *m)
{
    int i, expectedStart = 0;

    for (i = 0; i < m->count; i++)
    {
        if (m->blocks[i].start != expectedStart || m->blocks[i].size <= 0)
            return 0;
        if (i > 0 && m->blocks[i].isFree && m->blocks[i - 1].isFree)
            return 0;
        expectedStart += m->blocks[i].size;
    }
    return expectedStart == m->poolSize;
}

const char *strategyName(int strategy)
{
    if (strategy == BEST_FIT)
        return "Best Fit";
    if (strategy == WORST_FIT)
        return "Worst Fit";
    return "First Fit";
}

// Asks for a strategy again and again until the user enters 1, 2 or 3.
void chooseStrategy(MemoryManager *m)
{
    int choice, result;

    while (1)
    {
        printf("Select allocation strategy (1. First Fit, 2. Best Fit, "
               "3. Worst Fit): ");
        result = scanf("%d", &choice);

        if (result == EOF)
        {
            printf("\nInput ended. Exiting.\n");
            exit(0);
        }
        clearLine();

        if (result == 1 && choice >= 1 && choice <= 3)
            break;
        printf("Please enter 1, 2 or 3.\n");
    }

    m->strategy = choice;
    printf("Strategy set to %s.\n", strategyName(m->strategy));
}

void printMenu()
{
    printf("\nMenu:\n");
    printf("1. Allocate memory\n");
    printf("2. Release memory\n");
    printf("3. Resize allocation\n");
    printf("4. Display memory map\n");
    printf("5. Display memory statistics\n");
    printf("6. Change allocation strategy\n");
    printf("7. Exit\n");
}

void doAllocate(MemoryManager *m)
{
    int size;

    printf("Requested size: ");
    if (scanf("%d", &size) != 1)
    {
        clearLine();
        printf("Invalid size.\n");
        return;
    }
    clearLine();
    allocateMemory(m, size);
}

void doRelease(MemoryManager *m)
{
    int id;

    printf("Allocation ID: ");
    if (scanf("%d", &id) != 1)
    {
        clearLine();
        printf("Invalid ID.\n");
        return;
    }
    clearLine();
    releaseMemory(m, id);
}

void doResize(MemoryManager *m)
{
    int id, size;

    printf("Allocation ID: ");
    if (scanf("%d", &id) != 1)
    {
        clearLine();
        printf("Invalid input.\n");
        return;
    }
    clearLine();

    printf("New size: ");
    if (scanf("%d", &size) != 1)
    {
        clearLine();
        printf("Invalid input.\n");
        return;
    }
    clearLine();

    resizeAllocation(m, id, size);
}

// Main menu loop. Keeps running until the user picks option 7.
void runMenu(MemoryManager *m)
{
    int choice, result;

    do
    {
        printMenu();

        printf("Choice: ");
        result = scanf("%d", &choice);

        if (result == EOF)
        {
            printf("\nInput ended. Exiting.\n");
            exit(0);
        }
        clearLine();

        if (result != 1)
        {
            printf("Invalid input. Please enter a number from 1 to 7.\n");
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1:
            doAllocate(m);
            break;
        case 2:
            doRelease(m);
            break;
        case 3:
            doResize(m);
            break;
        case 4:
            displayMap(m);
            break;
        case 5:
            displayStats(m);
            break;
        case 6:
            chooseStrategy(m);
            break;
        case 7:
            printf("Exiting.\n");
            break;
        default:
            printf("Invalid choice. Please enter a number from 1 to 7.\n");
        }

        if (!mapIsConsistent(m))
            printf("Internal error: memory map is inconsistent!\n");
    } while (choice != 7);
}

int main()
{
    MemoryManager mgr;

    if (!initManager(&mgr, POOL_SIZE))
    {
        printf("Could not allocate memory for the pool. Exiting.\n");
        return 1;
    }
    printf("Memory pool created: %d bytes.\n", POOL_SIZE);

    chooseStrategy(&mgr);
    runMenu(&mgr);

    destroyManager(&mgr);
    return 0;
}