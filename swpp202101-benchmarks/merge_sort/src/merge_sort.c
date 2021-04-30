// https://www.geeksforgeeks.org/merge-sort/
#include <inttypes.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

void *malloc_upto_8(uint64_t x) {
  return malloc((x + 7) / 8 * 8);
}


void merge(int arr[], int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    /* create temp arrays */
    int *L = (int *)malloc_upto_8(sizeof(int) * n1);
    int *R = (int *)malloc_upto_8(sizeof(int) * n2);

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there
       are any */
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
}

int main() {
    uint64_t n = read();
    int *arr = (int *)malloc_upto_8(sizeof(int) * n);
    for (int i = 0; i < n; ++i)
        arr[i] = (int)read();

    mergeSort(arr, 0, n - 1);

    for (int i = 0; i < n; ++i) {
        write(arr[i]);
    }

    free(arr);

    return 0;
}
