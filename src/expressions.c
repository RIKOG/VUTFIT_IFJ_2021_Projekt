/**
 * Project: Compiler IFJ21
 *
 * @file expression.c
 *
 * @brief Implement functions processing the correct order of identificators in expressions
 *
 * @author Richard Gajdosik <xgajdo33>
 */

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "parser.h"
#include "error.h"
#include "expressions.h"
//typedef union token_attr {
//    string_t id;
//    uint64_t num_i;
//    double num_f;
//} token_attr_t;

//typedef struct token {
//    token_type_t type;
//    keywords_t keyword;
//    token_attr_t attr;
//} token_t;

//    11. <statement> → if <expression>  then <statement>  else
//        <statement>  end <statement>
//    12. <statement> → while <expression>  do <statement>  end
//                <statement>
//    15. <statement> → return <expression>  <other_exp>
//    <statement>
//    22. <type_expr> → <expression>  <other_exp>
//    23. <other_exp> → , <expression> <other_exp>
//    28. <init_assign> → <expression>

char Precedence_Table[][17] = {
        {"<>>>>>>>>>>>><><>"}, // #
        {"<>>>>>>>>>>>><><>"}, // *
        {"<>>>>>>>>>>>><><>"}, // /
        {"<>>>>>>>>>>>><><>"}, // //

        {"<<<<>>>>>>>>><><>"}, // +
        {"<<<<>>>>>>>>><><>"}, // -

        {"<<<<<<<>>>>>><><>"}, // ..
        {"<<<<<<<>>>>>><><>"}, // <<
        {"<<<<<<<>>>>>><><>"}, // <=
        {"<<<<<<<>>>>>><><>"}, // >>
        {"<<<<<<<>>>>>><><>"}, // >=
        {"<<<<<<<>>>>>><><>"}, // ==
        {"<<<<<<<>>>>>><><>"}, // ~=

        {"<<<<<<<<<<<<<<=<c"}, // (

        {">>>>>>>>>>>>>c>c>"}, // )

        {"c>>>>>>>>>>>>c>s>"}, // i

        {"<<<<<<<<<<<<<<c<c"} // $
};
char Chars[][3] = {
        {"#"}, {"*"}, {"/"}, {"//"},
        {"+"}, {"-"},
        {".."}, {"<"}, {"<="}, {">"}, {">="}, {"=="}, {"~="},
        {"("},
        {")"},
        {"i"},
        {"$"}
};

// We reversed the rules because when we copy from stack from top to bottom the expression (E) will become )E(
char Rules[][5] = {
        {"i"}, {")E("}, {"E+E"}, {"E-E"}, {"E*E"}, {"E/E"},
        {"E//E"}, {"E#"}, {"E<E"}, {"E=<E"}, {"E>E"}, {"E=>E"}, {"E==E"}, {"E=~E"}, {"E..E"}
};

// The first initialization, we create the stack and put $ as the first element of the stack
DLList * DLL_Init( DLList *list ) {
    // We create the list and check for malloc
    list = malloc(sizeof(DLList));
    if(list == NULL){
        //todo Call error handler
    }

    // We create the first element and check for malloc
    DLLElementPtr TempElement = malloc(sizeof(struct DLLElement));
    if (TempElement == NULL) {
        //todo Call error handler
    }

    // Because there is only one element, its both first and last one in the List
    list->firstElement = TempElement;
    list->lastElement = TempElement;

    // Because element is both first and last one in the List it has no previous nor next element
    list->firstElement->previousElement = NULL;
    list->firstElement->nextElement = NULL;

    // We add $ as the first element of stack
    strcpy(list->firstElement->data, "$\0");
    return list;
}
// We delete everything after Element with Element included
void DLL_Dispose( DLLElementPtr Element ) {
    if(Element == NULL){
        //todo Call error handler
        return;
    }

    DLLElementPtr TempElement = Element;
    DLLElementPtr DelElement;

    while(TempElement != NULL){
        DelElement = TempElement;
        TempElement = TempElement->nextElement;
        free(DelElement);
    }
}
// We are inserting << with its char (+, -, <= etc.)
void DLL_Insert(DLList *list, char * data ) {
    printf("insert: %s -> ", data);
    if(list == NULL){
        //todo Call error handler
        return;
    }
    int flag = 0;

    DLLElementPtr find = list->lastElement;
    // If stack contains for example $E and data contains +, we skip to previous element until we find one of our chars(+, -, <= etc.) and
    // connect the element in a way to create $<<E+,

    // otherwise if there is no E on top of stack, for example $<<E+, we just copy our << with data, $<<E+ -> $<<E+<<(
    // Whether there is E on top of the stack we check with int flag
    while((strcmp(find->data, "E") == 0) && find->previousElement != NULL){
        find = find->previousElement;
        flag = 1;
    }

    // We check if malloc was successful
    DLLElementPtr TempElement_first = malloc(sizeof(struct DLLElement));
    DLLElementPtr TempElement_second = malloc(sizeof(struct DLLElement));
    if (TempElement_first == NULL || TempElement_second == NULL) {
        //todo Call error handler
    }

    // If we found expression character on stack without going through E
    if(flag == 0){
        find->nextElement = TempElement_first;
        list->lastElement = TempElement_second;
        // We connect the chains
        // Found_Element -> first -> second -> NULL
        // $             -> <<    -> i      -> NULL
        TempElement_first->previousElement = find;
        TempElement_first->nextElement = TempElement_second;

        TempElement_second->previousElement = TempElement_first;
        TempElement_second->nextElement = NULL;

        strcpy(TempElement_first->data, "<<\0");
        strcpy(TempElement_second->data, data);
    // If we found E on stack, means we are copying << before E and data after E
    } else {
        DLLElementPtr Element_With_E = find->nextElement;

        find->nextElement = TempElement_first;
        list->lastElement = TempElement_second;
        // We connect the chains
        // Found_Element -> first -> E -> second -> NULL
        // $             -> <<    -> E -> +      -> NULL
        TempElement_first->previousElement = find;
        TempElement_first->nextElement = Element_With_E;

        Element_With_E->previousElement = TempElement_first;
        Element_With_E->nextElement = TempElement_second;

        TempElement_second->previousElement = Element_With_E;
        TempElement_second->nextElement = NULL;

        strcpy(TempElement_first->data, "<<\0");
        strcpy(TempElement_second->data, data);
    }
    print_stack_debug(list);
}
// We close the first part of the expression we find, for example <<E+<<i -> <<E+E
bool DLL_Close(DLList *list) {
    if(list == NULL){
        //todo Call error handler
        return false;
    }

    char Array_To_Check_Against_Rules[5] = {'\0'};

    DLLElementPtr find = list->lastElement;

    // We copy into our array until we dont find closing <<
    while((strcmp(find->data, "<<") != 0) && find->previousElement != NULL){

        strcat(Array_To_Check_Against_Rules, find->data);
        find = find->previousElement;
    }

    // We check against rules
    for(int j = 0; j < 15; j++){
        // If we found correct rule
        if(strcmp(Array_To_Check_Against_Rules, Rules[j]) == 0){
            // We delete everything after <<
            DLL_Dispose (find->nextElement);
            // We change << with E
            strcpy(find->data, "E\0");
            find->nextElement = NULL;
            list->lastElement = find;
            // We were successful in finding a rule
            return true;
        }
    }
    // We were not successful in finding a rule
    return false;
}
// Returns top of the stack
void DLL_Top(DLList *list, char data[]) {
    if(list == NULL){
        //todo Call error handler
        return;
    }
    DLLElementPtr find = list->lastElement;
    // If stack contains for example $E, we skip to previous element until we find one of our chars($, +, -, <= etc.) and
    while((strcmp(find->data, "E") == 0) && find->previousElement != NULL){
        find = find->previousElement;
    }
    strcpy(data, find->data);
}
// Copy a string on top of the stack
void DLL_Push(DLList *list, char * data) {
    if(list == NULL){
        //todo Call error handler
        return;
    }
    DLLElementPtr TempElement = malloc(sizeof(struct DLLElement));
    DLLElementPtr find = list->lastElement;

    find->nextElement = TempElement;
    // Connect the chains
    TempElement->previousElement = find;
    TempElement->nextElement = NULL;
    list->lastElement = TempElement;
    // Copy onto the stack
    strcpy(TempElement->data, data);
}
bool Check_Correct_Closure(DLList *list){
    if((strcmp(list->firstElement->data, "$") == 0) && (strcmp(list->lastElement->data, "E") == 0)){
        return true;
    }
    return false;
}
int Get_Index_Of_String(char * data){
    // if i remains 15 at the end of the for cycle, it means data contains not one of our chars (+, -, <= etc.) but a string or a variable
    int i = 15;
    for(int j = 0; j < 17; j++){
        if(strcmp(data, Chars[j]) == 0){
            return j;
        }
    }
    return i;
}
void print_stack_debug(DLList *list){
    DLLElementPtr PrintElement = list->firstElement;
    while(PrintElement != NULL) {
        printf("%s", PrintElement->data);
        PrintElement = PrintElement->nextElement;
    }
    printf("\n");
}
bool expression() {
    // todo get rid of magic numbers
    // todo conflicts with < char as in <i> and < as in f<5, solution, "<" = "<<"
    char data[3] = {"$\0"};
    DLList *list = NULL;
    list = DLL_Init(list);
    char precedence;
    while(TOKEN_ID_EXPRESSION()){
        xyz:
        // Look what char is on top of the stack (+, -, <= etc.)
        DLL_Top(list, data);

        // Check the characters precedence against the found token
        precedence = Precedence_Table[Get_Index_Of_String(data)][Get_Index_Of_String(token.attr.id.str)];
        printf("top nasiel: %s oproti tokenu: %s, precedencia: %c \n", data, token.attr.id.str, precedence);
        if(precedence == '<'){
            // If token is a variable or a string we put i on the stack instead of copying the whole name of the variable or whole string
            if(Get_Index_Of_String(token.attr.id.str) == 15){
                data[0] = 'i', data[1] = '\0';
            } else {
                // else we copy the character from the token (+, -, <= etc.)
                strcpy(data, token.attr.id.str);
            }
            // We copy the character from the token with << added infront of E $<<E+ or $<<E+<<( (+, -, <= etc.)
            DLL_Insert(list, data);
        } else if(precedence == '>'){
            // We find the first << from the top of the stack to the bottom and check it against the rules, if we didnt find a rule we return false as expression is wrong
            if(!DLL_Close(list)){
                return false;
            }
            print_stack_debug(list);
            // If we found a rule, we check the precedence of the new created stack by returning to the start of the while cycle
            goto xyz;
        }
            // We just copy the data onto the stack
        else if(precedence == '='){
            DLL_Push(list, token.attr.id.str);
        }
            // Special incident with finding identificator after identificator, which means the expression has ended and we close our stack and check against the rules
        else if(precedence == 's'){
            while(DLL_Close(list)){
                print_stack_debug(list);
            }
            if(Check_Correct_Closure(list)){
                printf("Vyraz je korektny.\n");
            }
            return Check_Correct_Closure(list);
        } else if(precedence == 'c'){
            return false;
        } else {

        }
        // Test print
//        printf("Precedence: %c%c TOKEN = \"%s\"\n", precedence, precedence, token.attr.id.str);
//        print_stack_debug(list);
        NEXT_TOKEN();
    }
    printf("\nwe are closing this:");
    print_stack_debug(list);
    while(DLL_Close(list)){
        print_stack_debug(list);
    }
    if(Check_Correct_Closure(list)){
        printf("Vyraz je korektny.\n");
    }
    return Check_Correct_Closure(list);
}
