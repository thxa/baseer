#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t data[5];
    size_t size;
} target_t;



void add_one(target_t *t) {
    for (size_t i = 0; i < t->size; i++)
        t->data[i] += 1;
    printf("[AddOne] Applied\n");
}

void multiply_two(target_t *t) {
    for (size_t i = 0; i < t->size; i++)
        t->data[i] *= 2;
    printf("[MultiplyTwo] Applied\n");
}

void subtract_one(target_t *t) {
    for (size_t i = 0; i < t->size; i++)
        t->data[i] -= 1;
    printf("[SubtractOne] Applied\n");
}

int main() {
    target_t t = {{1, 2, 3, 4, 5}, 5};

    printf("Original data: ");
    for (size_t i = 0; i < t.size; i++)
        printf("%d ", t.data[i]);
    printf("\n");

    // List of function pointers (pipeline)
    void (*pipeline[])(target_t *) = {add_one, multiply_two, subtract_one};
    size_t pipeline_size = sizeof(pipeline) / sizeof(pipeline[0]);

    // Execute each function in the pipeline
    for (size_t i = 0; i < pipeline_size; i++) {
        pipeline[i](&t);
    }

    printf("Modified data: ");
    for (size_t i = 0; i < t.size; i++)
        printf("%d ", t.data[i]);
    printf("\n");

    return 0;
}


