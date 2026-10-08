#include <stdio.h>


struct Student {
    int roll_no;
    char name[100];
    int m1;
    int m2;
    int m3;
    int Total;
    float average;
    char grade;
};

char gradeFunction(float avg) {
    if (avg >= 85) {
        return 'A';
    } else if (avg >= 70) {
        return 'B';
    } else if (avg >= 50) {
        return 'C';
    } else if (avg >= 35) {
        return 'D';
    } else {
        return 'F';
    }
}


void Print(int n){
    if(n==0){
        return;
    }else{
        
        Print(n-1);
        printf(" %d", n);
    }
}


int calcTotal(int a, int b, int c){
    return a + b + c;
}


float calcAverage(int total){ 
    return total / 3.0f;
}

int main(){

    int n;
    
    scanf("%d", &n);

    struct Student s[n];
    

    for(int i=0;i<n;i++){
        scanf("%d %s %d %d %d", &s[i].roll_no, s[i].name, &s[i].m1, &s[i].m2, &s[i].m3);
        s[i].Total = calcTotal(s[i].m1, s[i].m2, s[i].m3);
        s[i].average = calcAverage(s[i].Total);
        s[i].grade = gradeFunction(s[i].average);
    }

   printf("\n\n");

    for(int i=0;i<n;i++){
        printf("Roll: %d\nName: %s\nTotal: %d\nAverage: %.2f\nGrade: %c\n", s[i].roll_no, s[i].name, s[i].Total, s[i].average, s[i].grade);

        if(s[i].grade=='F'){
            printf("\n");
            continue;
        }
   
        printf("Performance:");

        if(s[i].grade=='A'){
            printf(" *****\n");
        }
        else if(s[i].grade=='B'){
            printf(" ****\n");
        }
        else if(s[i].grade=='C'){
            printf(" ***\n");
        }
        else if(s[i].grade=='D'){
            printf(" **\n");
        }
        printf("\n");
    }

    printf("List of Roll Numbers (via recursion):");
    Print(n);
    printf("\n");

}