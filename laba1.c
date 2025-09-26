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
    struct Elem *Next;  // указатель на следующий элемент
} Elem;

void add(int val, Elem*root) {
    Elem *last=root;
    Elem *new_v=(Elem*)malloc(sizeof(Elem));
    new_v->Val=val;
    new_v->Next=NULL;
    while(last->Next!=NULL) {
        last=last->Next;
    }
    last->Next=new_v;
}

int get(int index, Elem *root);{
    Elem *last=root;
    for (int i = 0; i <= index;i++) {
        last = last->Next;
    }
    return last->Val;
}
main(){
    Elem * root = (Elem*)malloc(sizeof(Elem));
    root->Next=NULL;
    add(5);
    add(6);
    printf("Val -> %d\n", get(3, root));
    get();
}
