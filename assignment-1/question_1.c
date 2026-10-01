#include <stdio.h>
#include <ctype.h>
#define MAX 200
int main()
{

    char input[MAX];

    printf("Input: ");
    fgets(input, MAX, stdin);

    int numC = 0, opC = 0;
    int numArr[100], opArr[100];
    int next = 1;
int neg=0;
    int i = 0;
    while (input[i] != '\0' && input[i] != '\n')
    {

        while (input[i] == ' ')
        {
            i++;
        }
        if (input[i] == '\0' || input[i] == '\n')
            break;

        if (isdigit(input[i]))
        {

            if (!next)
            {
                printf("Error: Invalid expression.\n");
                return 1;
            }
            next = 0;

            int temp = 0;
            while (isdigit(input[i]))
            {
                temp = temp * 10 + (input[i] - '0');
                i++;
            }

            if(neg)
            {
                temp = -temp;
                neg = 0;
            }
            numArr[numC] = temp;
            numC++;
        }
        else if (input[i] == '+' || input[i] == '-' || input[i] == '*' || input[i] == '/')
        {
            if (next)
            {

                if(input[i] == '-' )
                {i++;
                    while (input[i] == ' ')
                    {
                        i++;
                    }

                    if(isdigit(input[i]))
                    {
                    neg = 1;
                    
                    continue;}
                }


                printf("Error: Invalid expression.\n");
                return 1;
            }
            next = 1;
            opArr[opC] = input[i];
            opC++;
            i++;
        }
        else
        {
            printf("Error: Invalid expression.\n");
            return 1;
        }
    }

    if (next)
    {
        printf("Error: Invalid expression.\n");
        return 1;
    }

    int newNum[100], newOp[100];
    int n = 0, o = 0;

    newNum[n] = numArr[0];
    n++;

    for (int i = 0; i < opC; i++)
    {
        if (opArr[i] == '*')
        {
            newNum[n - 1] *= numArr[i + 1];
        }
        else if (opArr[i] == '/')
        {
            if (numArr[i + 1] == 0)
            {
                printf("Error: Division by zero.\n");
                return 1;
            }

            newNum[n - 1] /= numArr[i + 1];
        }
        else
        {
            newOp[o] = opArr[i];
            o++;
            newNum[n] = numArr[i + 1];
            n++;
        }
    }

    int result = newNum[0];
    for (int i = 0; i < o; i++)
    {
        if (newOp[i] == '+')
        {
            result += newNum[i + 1];
        }
        else if (newOp[i] == '-')
        {
            result -= newNum[i + 1];
        }
    }
    printf("Output: %d\n", result);

    return 0;
}