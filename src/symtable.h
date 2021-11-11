/**
 * Project: Compiler IFJ21
 *
 * @file symtable.h
 *
 * @brief Contain functions declaration and data types representing
 *        a symbol table and working with it using hash table.
 *
 * @author Andrei Shchapaniak <xshcha00>
 */

#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <stdbool.h>
#include <stdint.h>
#include "scanner.h"
#include "str.h"

#define MAX_HT_SIZE 101

#define DEL_SYMTAB() \
        delete_last_symtab(&local_symtbs)

#define ADD_SYMTAB() \
        ret = add_symtab(&local_symtbs); \
        if (!ret) { \
            return false; \
        }

#define ADD_FUNC_TO_SYMTAB() \
        if (!symtab_add(&global_symtab, &token.attr.id)) { \
            return false; \
        }

#define FIND_VAR_IN_SYMTAB find_id_symtbs(&local_symtbs, token.attr.id.str)

#define FIND_FUNC_IN_SYMTAB symtab_find(&global_symtab, token.attr.id.str)

#define CHECK_ID(EL) \
        if (FIND_##EL##_IN_SYMTAB == false) { \
            err = SEM_DEF_ERR; \
            return false; \
        }


#define ADD_VAR_TO_SYMTAB() \
        if (!symtab_add(&local_symtbs.htab[local_symtbs.size - 1], &token.attr.id)) { \
            return false; \
        }

typedef enum type_id {
    VAR,
    FUNC
} type_id_t;

typedef enum type {
    STRING,
    INTEGER,
    NUMBER,
    NIL
} type_t;

typedef struct var {
    bool init;
    bool val_nil;
    type_t type;
    token_type_t attr;
} var_t;

typedef struct func {
    bool def;
    bool decl;
    type_t *types;
    type_t *rets;
} func_t;

typedef union data {
    var_t *var;
    func_t *func;
} data_t;

typedef struct htab_item {
    char* key_id;
    type_id_t type;
    data_t data;
    struct htab_item *next;
} htab_item_t;

typedef htab_item_t *htable_t[MAX_HT_SIZE];

typedef struct {
    int size;
    htable_t *htab;
} arr_symtbs_t;

/*
 *
 */
void free_symtbs(arr_symtbs_t *symtbs);

/*
 *
 */
void delete_last_symtab(arr_symtbs_t *symtbs);

/*
 *
 */
bool add_symtab(arr_symtbs_t *symtbs);

/*
 *
 */
bool find_id_symtbs(arr_symtbs_t *symtbs, const char *key);

/*
 * @brief Sdbm algorithm for hash table
 * @return Hash of the string
 */
uint32_t symtab_hash(const char *id);

/*
 * @brief Allocate hash table and set the structure values
 */
void symtab_init(htable_t *table);

/*
 * @brief Find the item int the table that matches the key
 * @param table - symbol table
 * @param key - key for searching item
 * @return On success pointer to item, otherwise NULL
 */
htab_item_t *symtab_find(const htable_t *table, const char *key);

/*
 * @brief Create new item in the table
 * @return Pointer to the created item, otherwise NULL
 */
htab_item_t *symtab_add(htable_t *table, const string_t *s);

bool symtab_add_params(htable_t *table, token_t *token, bool value);

/*
 * @brief Table destructor
 */
void symtab_free(htable_t *table);

#endif // _SYMTABLE_H
