// struct Point {
//     int x;
//     int y;
// };Point;
//
// typedef struct Geo {
//     char*name;
// };
//
// #include <stdio.h>
// #include <stdlib.h>
//
// // Определяем структуру Elem
// typedef struct Elem {
//     int Val;
//     struct Elem *Next;  // указатель на следующий элемент
// } Elem;
//
// int main() {
//     // Создаём два элемента
//     Elem a = {10, NULL};
//     Elem b = {20, NULL};
//
//     // Связываем их
//     a.Next = &b;
//
//     // Выводим цепочку
//     printf("%d -> %d\n", a.Val, a.Next->Val);
//
//     return 0; // завершение программы
// }


#include <stdio.h>
#include <stdlib.h>

typedef struct Elem {
    int Val;
    struct Elem *Next;
} Elem;

// добавление в хвост списка с фиктивной головой root
void add(int val, Elem *root) {
    Elem *last = root;
    while (last->Next != NULL) {
        last = last->Next;
    }
    Elem *new_v = (Elem*)malloc(sizeof(Elem));
    if (!new_v) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_v->Val = val;
    new_v->Next = NULL;
    last->Next = new_v;
}

// получить значение по индексу (0 — первый реальный элемент)
int get(int index, Elem *root) {
    if (index < 0) {
        fprintf(stderr, "Negative index\n");
        exit(EXIT_FAILURE);
    }
    Elem *node = root->Next;  // пропускаем фиктивную голову
    int i = 0;
    while (node != NULL && i < index) {
        node = node->Next;
        i++;
    }
    if (node == NULL) {
        fprintf(stderr, "Index %d out of range\n", index);
        exit(EXIT_FAILURE);
    }
    return node->Val;
}

int main(void) {
    // создаём фиктивную голову
    Elem *root = (Elem*)malloc(sizeof(Elem));
    if (!root) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    root->Next = NULL;

    add(5, root);
    add(6, root);

    // 0 -> первый элемент (5), 1 -> второй элемент (6)
    printf("Val -> %d\n", get(1, root));

    // освобождение памяти
    Elem *cur = root;
    while (cur) {
        Elem *next = cur->Next;
        free(cur);
        cur = next;
    }
    return 0;
}
