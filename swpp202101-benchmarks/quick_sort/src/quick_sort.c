// https://www.geeksforgeeks.org/quick-sort/
#include <inttypes.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

void *malloc_upto_8(uint64_t x) {
  return malloc((x + 7) / 8 * 8);
}


void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than the pivot
        if (arr[j] < pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSort(int arr[], int low, int high) {
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}


int main() {
    uint64_t n = read();
    int *arr = (int *)malloc_upto_8(sizeof(int) * n);
    for (int i = 0; i < n; ++i)
        arr[i] = (int)read();

    quickSort(arr, 0, n - 1);

    for (int i = 0; i < n; ++i) {
        write(arr[i]);
    }

    free(arr);

    return 0;
}
