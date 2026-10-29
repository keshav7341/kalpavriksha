#include<stdio.h>
#include<stdlib.h>


void display(int n,int *mat){

    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            printf("%d  ",*(mat+i*n+j));
        }
        printf("\n");
    }
}

void transpose(int n,int *mat);
void rotate(int n,int *mat);
void swap(int *a,int *b);


void clockwise(int n,int *mat){
    transpose(n,mat);
    rotate(n,mat);
}


// swap element (i,j) with element (j,i)
void transpose(int n,int *mat){
    for(int i=0;i<n;i++){
        for(int j=i;j<n;j++){
            if(i==j) continue;

            swap(mat+i*n+j,mat+j*n+i);

        }
    }
}


// reverses one row, mat points to the start of that row
void row_rotate(int *mat,int n){
    int l=0,r=n-1;
    while(l<r){
        swap(mat+l++,mat+r--);
    }
}


// reverse every row of the transposed matrix
void rotate(int n,int *mat){
    for(int i=0;i<n;i++){
       row_rotate(mat+i*n,n);
    }
}


void swap(int *a,int *b){
    *a=*a^*b;
    *b=*a^*b;
    *a=*a^*b;
}

void allocateValues(int n,int *mat){
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            *(mat+i*n+j)=rand()%256;
        }
    }
}


// smoothing filter, result is written back into the same matrix
// if we overwrite a cell right away, the next cell would use the new value
// instead of the original one, so we keep original copies of the row above
// (prev) and the current row (cur). the row below is not changed yet so we
// read it directly from the matrix. extra space used is only 2 rows, O(N)
void smooth(int n,int *mat){
    int *prev=(int *)malloc(n*sizeof(int));
    int *cur=(int *)malloc(n*sizeof(int));

    // stop if memory was not allocated
    if(prev==NULL || cur==NULL){
        printf("Memory allocation failed\n");
        free(prev);
        free(cur);
        prev=NULL;
        cur=NULL;
        return;
    }

    int *rowp=mat;      // start of the current row

    for(int i=0;i<n;i++,rowp+=n){

        // copy the original values of this row into cur
        for(int c=0;c<n;c++){
            *(cur+c)=*(rowp+c);
        }

        for(int j=0;j<n;j++){
            int sum=0,cnt=0;

            // nr is the neighbour row, nc is the neighbour column
            for(int nr=i-1;nr<=i+1;nr++){
                if(nr<0 || nr>=n) continue;     // row outside the matrix

                // pick the row which still has original values
                int *src;
                if(nr==i-1) src=prev;
                else if(nr==i) src=cur;
                else src=rowp+n;

                for(int nc=j-1;nc<=j+1;nc++){
                    if(nc<0 || nc>=n) continue; // column outside the matrix
                    sum+=*(src+nc);
                    cnt++;
                }
            }

            *(rowp+j)=sum/cnt;
        }

        // current row becomes the previous row for the next iteration
        int *t=prev;
        prev=cur;
        cur=t;
    }

    free(prev);
    free(cur);
    prev=NULL;
    cur=NULL;
}


int main(){

    // n is size of square matrix
    int n;
    printf("Enter matrix size (2-10):");
    scanf("%d",&n);

    if(n<2 || n>10){
        printf("Size must be between 2 and 10\n");
        return 1;
    }

    // mat is for matrix
    int *mat=(int *)malloc(n*n*sizeof(int));
    if(mat==NULL){
        printf("Memory allocation failed\n");
        return 1;
    }

    
    //to allocate random values to matrix
    allocateValues(n,mat);

    printf("\nOriginal Matrix:\n");
    display(n,mat);

    clockwise(n,mat);
    printf("\nMatrix after Rotation:\n");
    display(n,mat);

    smooth(n,mat);
    printf("\nMatrix after Applying Filter:\n");
    display(n,mat);

    free(mat);
    // to avoid dangling pointer, set mat to NULL after freeing
    mat=NULL;
    return 0;

}