#include <inttypes.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

extern uint64_t* root;

uint64_t insert(uint64_t data) {
  if (root == NULL) {
    uint64_t* node = (uint64_t*)malloc(3 * sizeof(uint64_t));
    *node = data;
    *(node + 1) = (uint64_t)NULL;
    *(node + 2) = (uint64_t)NULL;
    root = node;
    return 1;
  }

  uint64_t* curr = root;

  while (1) {
    uint64_t curr_data = *curr;
    uint64_t* next;

    if (curr_data > data) {
      next = (uint64_t*)*(curr + 1);
      if (next == NULL) {
        uint64_t* node = (uint64_t*)malloc(3 * sizeof(uint64_t));
        *node = data;
        *(node + 1) = (uint64_t)NULL;
        *(node + 2) = (uint64_t)NULL;
        *(curr + 1) = (uint64_t)node;
        return 1;
      }
      curr = next;
      continue;
    }
    else if (curr_data < data) {
      next = (uint64_t*)*(curr + 2);
      if (next == NULL) {
        uint64_t* node = (uint64_t*)malloc(3 * sizeof(uint64_t));
        *node = data;
        *(node + 1) = (uint64_t)NULL;
        *(node + 2) = (uint64_t)NULL;
        *(curr + 2) = (uint64_t)node;
        return 1;
      }
      curr = next;
      continue;
    }
    else return 0;
  }
}

uint64_t adjust(uint64_t* node) {
  uint64_t* parent = node;
  uint64_t* curr = (uint64_t*)*(node + 1);

  while (1) {
    uint64_t data = *curr;
    uint64_t* left = (uint64_t*)*(curr + 1);
    uint64_t* right = (uint64_t*)*(curr + 2);

    if (right == NULL) {
      if (curr == (uint64_t*)*(parent + 1))
        *(parent + 1) = (uint64_t)left;
      else
        *(parent + 2) = (uint64_t)right;
     
      free(curr);
      return data;
    }

    parent = curr;
    curr = right;
  }
}

uint64_t remove(uint64_t data) {
  if (root == NULL)
    return 0;

  uint64_t* parent = NULL;
  uint64_t* curr = root;

  while (1) {
    if (curr == NULL)
      return 0;

    uint64_t curr_data = *curr;
    uint64_t* left = (uint64_t*)*(curr + 1);
    uint64_t* right = (uint64_t*)*(curr + 2);

    if (data < curr_data) {
      parent = curr;
      curr = (uint64_t*)*(curr + 1);
      continue;
    }

    if (data > curr_data) {
      parent = curr;
      curr = (uint64_t*)*(curr + 2);
      continue;
    }

    if (left == NULL) {
      if (parent == NULL)
        root = right;
      else if (curr == (uint64_t*)*(parent + 1))
        *(parent + 1) = (uint64_t)right;
      else
        *(parent + 2) = (uint64_t)right;

      free(curr);
      return 1;
    }

    if (right == NULL) {
      if (parent == NULL)
        root = left;
      else if (curr == (uint64_t*)*(parent + 1))
        *(parent + 1) = (uint64_t)left;
      else
        *(parent + 2) = (uint64_t)left;

      free(curr);
      return 1;
    }

    uint64_t new_data = adjust(curr);
    *curr = new_data;

    return 1;
  }
}

void traverse(uint64_t* node) {
  if (node == NULL) return;

  uint64_t data = *node;
  uint64_t* left = (uint64_t*)*(node + 1);
  uint64_t* right = (uint64_t*)*(node + 2);

  traverse(left);
  write(data);
  traverse(right);

  return;
}

int main() {
  root = NULL;
  
  uint64_t n = read();
  for (uint64_t i = 0; i < n; i++) {
    uint64_t action = read();
    uint64_t data = read();

    if (action == 0)
      insert(data);
    else
      remove(data);
  }

  traverse(root);
}
