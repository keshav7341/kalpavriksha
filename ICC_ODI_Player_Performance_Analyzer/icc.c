#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//to include data from the file
#include "Players_data.h"

//Player Node
typedef struct PlayerNode
{
    int playerId;
    char name[51];
    char teamName[51];
    char role[20];

    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;

    struct PlayerNode *next;
} PlayerNode;

typedef struct
{
    int teamId;
    char name[51];

    int totalPlayers;
    float averageBattingStrikeRate;

    float totalBattingStrikeRate;
    int battingPlayerCount;

    PlayerNode *batsmen;
    PlayerNode *bowlers;
    PlayerNode *allRounders;
} Team;

// 10 teams are given 
Team teamList[10];

// function to display menu in main function
void displayMenu()
{
    printf("\n============================================================\n");
    printf(" ICC ODI Player Performance Analyzer\n");
    printf("============================================================\n\n");

    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");

    printf("\n============================================================\n");
}

void printLine()
{
    printf("=========================================================================================================\n");
}

void printTableHeader()
{
    printLine();
    printf("%-6s %-25s %-14s %-12s %6s %6s %7s %5s %5s %10s\n", "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printLine();
}

void printPlayer(PlayerNode *player)
{
    printf("%-6d %-25s %-14s %-12s %6d %6.1f %7.1f %5d %5.1f %10.2f\n", player->playerId, player->name, player->teamName, player->role, player->totalRuns, player->battingAverage, player->strikeRate, player->wickets, player->economyRate, player->performanceIndex);
}

void printPlayerList(PlayerNode *head)
{
    while (head != NULL)
    {
        printPlayer(head);
        head = head->next;
    }
}

/* Converts menu choice (1, 2, 3) to role name */
const char *getRole(int choice)
{
    if (choice == 1)
        return "Batsman";
    if (choice == 2)
        return "Bowler";
    if (choice == 3)
        return "All-rounder";
    return NULL;
}

int readRole()
{
    int choice;
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &choice);
    return choice;
}

/* Returns the list (batsmen / bowlers / allRounders) of a team for a role */
PlayerNode **getRoleList(Team *team, const char *role)
{
    if (strcmp(role, "Batsman") == 0)
        return &team->batsmen;
    if (strcmp(role, "Bowler") == 0)
        return &team->bowlers;
    return &team->allRounders;
}

void initializeTeams()
{
    for (int i = 0; i < teamCount; i++)
    {
        teamList[i].teamId = i + 1;
        strcpy(teamList[i].name, teams[i]);

        teamList[i].totalPlayers = 0;
        teamList[i].averageBattingStrikeRate = 0;
        teamList[i].totalBattingStrikeRate = 0;
        teamList[i].battingPlayerCount = 0;

        teamList[i].batsmen = NULL;
        teamList[i].bowlers = NULL;
        teamList[i].allRounders = NULL;
    }
}

/* Binary search by team id - O(log n) */
Team *findTeamById(int teamId)
{
    int low = 0, high = teamCount - 1;

    while (low <= high)
    {
        int mid = (low + high) / 2;

        if (teamList[mid].teamId == teamId)
            return &teamList[mid];
        else if (teamList[mid].teamId < teamId)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return NULL;
}

/* Binary search by team name - O(log n) (team names are in alphabetical order) */
Team *findTeamByName(const char *teamName)
{
    int low = 0, high = teamCount - 1;

    while (low <= high)
    {
        int mid = (low + high) / 2;
        int cmp = strcmp(teamList[mid].name, teamName);

        if (cmp == 0)
            return &teamList[mid];
        else if (cmp < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return NULL;
}

Team *readTeam()
{
    int teamId;
    printf("Enter Team ID: ");
    scanf("%d", &teamId);

    Team *team = findTeamById(teamId);
    if (team == NULL)
        printf("Team not found\n");
    return team;
}

float calculatePI(PlayerNode *player)
{
    if (strcmp(player->role, "Batsman") == 0)
        return (player->battingAverage * player->strikeRate) / 100;

    if (strcmp(player->role, "Bowler") == 0)
        return (player->wickets * 2) + (100 - player->economyRate);

    return ((player->battingAverage * player->strikeRate) / 100) + (player->wickets * 2);
}

PlayerNode *createPlayer(int id, const char *name, const char *team, const char *role, int runs, float avg, float sr, int wickets, float er)
{
    PlayerNode *newNode = malloc(sizeof(PlayerNode));
    if (newNode == NULL)
    {
        printf("Memory Not Allocated\n");
        return NULL;
    }

    newNode->playerId = id;
    strcpy(newNode->name, name);
    strcpy(newNode->teamName, team);
    strcpy(newNode->role, role);

    newNode->totalRuns = runs;
    newNode->battingAverage = avg;
    newNode->strikeRate = sr;
    newNode->wickets = wickets;
    newNode->economyRate = er;
    newNode->performanceIndex = calculatePI(newNode);
    newNode->next = NULL;

    return newNode;
}

/* Inserts player so the list stays sorted by performance index (descending) */
void insertPlayerSorted(PlayerNode **head, PlayerNode *newNode)
{
    if (*head == NULL || newNode->performanceIndex > (*head)->performanceIndex)
    {
        newNode->next = *head;
        *head = newNode;
        return;
    }

    PlayerNode *temp = *head;
    while (temp->next != NULL && temp->next->performanceIndex >= newNode->performanceIndex)
    {
        temp = temp->next;
    }

    newNode->next = temp->next;
    temp->next = newNode;
}

/* Adds player to the correct role list and updates team stats */
void addPlayerToTeam(Team *team, PlayerNode *player)
{
    insertPlayerSorted(getRoleList(team, player->role), player);
    team->totalPlayers++;

    if (strcmp(player->role, "Batsman") == 0 || strcmp(player->role, "All-rounder") == 0)
    {
        team->totalBattingStrikeRate += player->strikeRate;
        team->battingPlayerCount++;
        team->averageBattingStrikeRate = team->totalBattingStrikeRate / team->battingPlayerCount;
    }
}

void initializePlayers()
{
    for (int i = 0; i < playerCount; i++)
    {
        Team *team = findTeamByName(players[i].team);
        if (team == NULL)
        {
            printf("Team not found for %s\n", players[i].name);
            continue;
        }

        PlayerNode *newNode = createPlayer(players[i].id, players[i].name, players[i].team, players[i].role, players[i].totalRuns, players[i].battingAverage, players[i].strikeRate, players[i].wickets, players[i].economyRate);
        if (newNode == NULL)
            exit(1);

        addPlayerToTeam(team, newNode);
    }
}

/* 1. Add Player */
void addPlayer()
{
    int teamId, playerId, roleChoice, totalRuns, wickets;
    float battingAverage, strikeRate, economyRate;
    char name[51];

    printf("Enter Team ID to add player: ");
    scanf("%d", &teamId);

    Team *team = findTeamById(teamId);
    if (team == NULL)
    {
        printf("Team not found\n");
        return;
    }

    printf("\nEnter Player Details:\n\n");

    printf("Player ID: ");
    scanf("%d", &playerId);

    printf("Name: ");
    scanf(" %50[^\n]", name);

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &roleChoice);
    if (getRole(roleChoice) == NULL)
    {
        printf("Invalid role\n");
        return;
    }

    printf("Total Runs: ");
    scanf("%d", &totalRuns);

    printf("Batting Average: ");
    scanf("%f", &battingAverage);

    printf("Strike Rate: ");
    scanf("%f", &strikeRate);

    printf("Wickets: ");
    scanf("%d", &wickets);

    printf("Economy Rate: ");
    scanf("%f", &economyRate);

    PlayerNode *newNode = createPlayer(playerId, name, team->name, getRole(roleChoice), totalRuns, battingAverage, strikeRate, wickets, economyRate);
    if (newNode == NULL)
        return;

    addPlayerToTeam(team, newNode);
    printf("\nPlayer added successfully to Team %s!\n", team->name);
}

/* 2. Display all players of a team */
void displayTeamPlayers()
{
    Team *team = readTeam();
    if (team == NULL)
        return;

    printf("\nPlayers of Team %s:\n", team->name);
    printTableHeader();
    printPlayerList(team->batsmen);
    printPlayerList(team->bowlers);
    printPlayerList(team->allRounders);
    printLine();

    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

/* 3. Display teams by average batting strike rate (descending) */
void displayTeamsByStrikeRate()
{
    Team *sorted[10];
    for (int i = 0; i < teamCount; i++)
        sorted[i] = &teamList[i];

    /* Insertion sort - only 10 teams */
    for (int i = 1; i < teamCount; i++)
    {
        Team *key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j]->averageBattingStrikeRate < key->averageBattingStrikeRate)
        {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("%-5s %-18s %-14s %s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("=========================================================\n");
    for (int i = 0; i < teamCount; i++)
        printf("%-5d %-18s %-14.1f %d\n", sorted[i]->teamId, sorted[i]->name, sorted[i]->averageBattingStrikeRate, sorted[i]->totalPlayers);
    printf("=========================================================\n");
}

/* 4. Top K players of a team by role - O(K) because the list is already sorted */
void displayTopKPlayers()
{
    Team *team = readTeam();
    if (team == NULL)
        return;

    const char *role = getRole(readRole());
    if (role == NULL)
    {
        printf("Invalid role\n");
        return;
    }

    int k;
    printf("Enter number of players: ");
    scanf("%d", &k);

    printf("\nTop %d %s of Team %s:\n", k, role, team->name);
    printTableHeader();

    PlayerNode *temp = *getRoleList(team, role);
    for (int i = 0; i < k && temp != NULL; i++)
    {
        printPlayer(temp);
        temp = temp->next;
    }
    printLine();
}

/* Max-heap helper: moves the node at index down to its correct place */
void heapify(PlayerNode *heap[], int size, int index)
{
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < size && heap[left]->performanceIndex > heap[largest]->performanceIndex)
        largest = left;
    if (right < size && heap[right]->performanceIndex > heap[largest]->performanceIndex)
        largest = right;

    if (largest != index)
    {
        PlayerNode *temp = heap[index];
        heap[index] = heap[largest];
        heap[largest] = temp;
        heapify(heap, size, largest);
    }
}

// 5. All players of a role across all teams - O(N log t) Each team's list is already sorted, so we merge the 10 lists using a max-heap. 
void displayPlayersByRole()
{
    const char *role = getRole(readRole());
    if (role == NULL)
    {
        printf("Invalid role\n");
        return;
    }

    PlayerNode *heap[10];
    int size = 0;

    for (int i = 0; i < teamCount; i++)
    {
        PlayerNode *head = *getRoleList(&teamList[i], role);
        if (head != NULL)
            heap[size++] = head;
    }

    for (int i = size / 2 - 1; i >= 0; i--)
        heapify(heap, size, i);

    printf("\n%s of all teams:\n", role);
    printTableHeader();

    while (size > 0)
    {
        PlayerNode *best = heap[0];
        printPlayer(best);

        if (best->next != NULL)
            heap[0] = best->next;
        else
            heap[0] = heap[--size];

        heapify(heap, size, 0);
    }
    printLine();
}

void freeList(PlayerNode *head)
{
    while (head != NULL)
    {
        PlayerNode *temp = head;
        head = head->next;
        free(temp);
    }
}

void freeAll()
{
    for (int i = 0; i < teamCount; i++)
    {
        freeList(teamList[i].batsmen);
        freeList(teamList[i].bowlers);
        freeList(teamList[i].allRounders);
    }
}

int main()
{
    int choice;

    initializeTeams();
    initializePlayers();

    do
    {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            addPlayer();
            break;

        case 2:
            displayTeamPlayers();
            break;

        case 3:
            displayTeamsByStrikeRate();
            break;

        case 4:
            displayTopKPlayers();
            break;

        case 5:
            displayPlayersByRole();
            break;

        case 6:
            printf("\nExit\n");
            break;

        default:
            printf("\nEnter Correct Choice\n");
            break;
        }
    } while (choice != 6);

    freeAll();
    return 0;
}