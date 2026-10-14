#include <stdio.h>

const int STANDARD_TICKET_PRICE = 150;
const int PREMIUM_TICKET_PRICE = 250;
const int RECLINER_TICKET_PRICE = 400;

const int MAX_TICKETS = 20;

int main()
{

    // to store customer name
    char CustomerName[100];

    // since the tickets boocked is between 1 to 20
    short int numberOfTickets;

    // ticket type: 1 = Standard, 2 = Premium, 3 = Recliner
    short int ticketType;

    // input format will be: CustomerName, numberOfTickets, ticketType all in different lines

    scanf("%99s", CustomerName); // there will be no space in name as it is mentioned in assignment
    scanf("%hd", &numberOfTickets);
    if (numberOfTickets < 1 || numberOfTickets > MAX_TICKETS)
    {
        printf("Invalid number of tickets. Please enter a value between 1 and 20.\n");
        return 1; // Exit the program
    }

    // ticket type: 1 = Standard, 2 = Premium, 3 = Recliner
    scanf("%hd", &ticketType);

    int costOFSingleTicket = 0;
    switch (ticketType)
    {
    case 1:
        costOFSingleTicket = STANDARD_TICKET_PRICE;
        break;
    case 2:
        costOFSingleTicket = PREMIUM_TICKET_PRICE;
        break;
    case 3:
        costOFSingleTicket = RECLINER_TICKET_PRICE;
        break;
    default:
        printf("Invalid ticket type. Please enter 1 for Standard, 2 for Premium, or 3 for Recliner.\n");
        return 1; // Exit the program
    }

    float subTotal = costOFSingleTicket * numberOfTickets;
    float discount = 0.0;
    if (numberOfTickets >= 5)
    {
        discount = subTotal * 0.10; // 10% discount for 5 or more tickets
    }
    float totalCost = subTotal - discount;

    // output

    printf("Customer: %s\n", CustomerName);

    printf("Ticket Category: ");
    switch (ticketType)
    {
    case 1:
        printf("Standard\n");
        break;
    case 2:
        printf("Premium\n");
        break;
    case 3:
        printf("Recliner\n");
        break;
    }
    printf("Tickets: %d\n", numberOfTickets);
    printf("Subtotal: %.2f\n", subTotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", totalCost);

    return 0;
}