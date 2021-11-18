/**
 * Project: Compiler IFJ21
 *
 * @file parser.h
 *
 * @brief Contain functions declaration for processing rules based on LL-table.
 *
 * @author Andrei Shchapaniak <xshcha00>
 */

#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include "scanner.h"
#include "symtable.h"

#define COUNT_DEF_FUNCS 8

// If you want to see all rules, which are used for process your program
// Pass RULES to cmake or read README.md
#ifdef DEBUG_RULES
    #define print_rule(s) printf("%s\n", (s))
#else
    #define print_rule(s)
#endif

// Get next token and check for INTERNAL/SCANNER error
#define NEXT_TOKEN() \
        err = get_next_token(&token); \
        if (err != NO_ERR) { \
            return false; \
        }

// If condition is false, PARSER error
#define EXPECTED_TOKEN(cond) \
        if (!(cond)) { \
            return false; \
        }

// Call function for expected nonterminal,
// in case of error return false
#define NEXT_NONTERM(nonterm) \
        ret = (nonterm); \
        if (!ret) { \
            return false; \
        }

// Get the first token from code
#define FIRST_TOKEN() \
        err = get_next_token(&token); \
        if (err != NO_ERR) { \
            goto end_parser; \
        }

// Check if type of token is id or some term
#define TOKEN_TERM() token.keyword == KW_NIL || \
                     token.type == T_INT     || \
                     token.type == T_FLOAT   || \
                     token.type == T_STRING

extern token_t token;
extern int err;

/*
 * @brief process start of the code
 *
 * 1. <prolog> -> require term_str <prog>
 */
bool prolog();

/*
 * @brief process all rules start with nonterminal <prog>
 *
 * 2. <prog> -> GLOBAL ID : FUNCTION ( <type_params> ) <type_returns> <prog>
 * 3. <prog> -> FUNCTION ID ( <params> ) <type_returns> <statement> <return_type> END <prog>
 * 4. <prog> -> ID_FUNC ( <args> ) <prog>
 * 5. <prog> -> EOF
 */
bool prog();

/*
 * @brief process all possible types of variables
 *
 * 6. <type> -> integer
 * 7. <type> -> number
 * 8. <type> -> string
 * 9. <type> -> nil
 */
bool type();

/*
 * @brief process all rules start with nonterminal <statement>
 *
 * 10. <statement> -> IF <expression> THEN <statement> ELSE <statement> END <statement>
 * 11. <statement> -> WHILE <expression> DO <statement> END <statement>
 * 12. <statement> -> LOCAL ID_VAR : <type> <def_var> <statement>
 * 13. <statement> -> ID_FUNC ( <args> ) <statement>
 * 14. <statement> -> ID_VAR <vars> <statement>
 * 15. <statement> -> RETURN <expression> <other_exp> <statement>
 * 16. <statement> -> e
 */
bool statement();

/*
 * @brief process rules for multiply assigning
 *
 * 17. <vars> -> , ID_VAR <vars>
 * 18. <vars> -> = <type_expr>
 */
bool vars();

/*
 * @brief process rules for assigning
 *
 * 19. <type_expr> -> ID_FUNC ( <args> )
 * 20. <type_expr> -> <expression> <other_exp>
 */
bool type_expr();

/*
 * @brief process myltiply assigning
 *
 * 21. <other_exp> -> , <expression> <other_exp>
 * 22. <other_exp> -> e
 */
bool other_exp();

/*
 * @brief process rules for definition of variables
 *
 * 23. <def_var> -> <init_assign>
 * 24. <def_var> -> e
 */
bool def_var();

/*
 * @brief process rules for the first initializing of variables
 *
 * 25. <init_assign> -> ID_FUNC ( <args> )
 * 26. <init_assign> -> <expression>
 */
bool init_assign();

/*
 * @brief process all possible types of variables in function return
 *
 * 27. <type_returns> -> : <type> <other_types_returns>
 * 28. <type_returns> -> e
 */
bool type_returns();

/*
 * @brief process all possible types of variables in function return
 *
 * 29. <other_types_returns> -> , <type> <other_types_returns>
 * 30. <other_types_returns> -> e
 */
bool other_types_returns();

/*
 * @brief process all possible types of variables in function arguments
 *
 * 31. <other_types_params> -> , <type> <other_types_params>
 * 32. <other_types_params> -> e
 */
bool other_types_params();

/*
 * @brief process params as arguments for function
 *
 * 33. <params> -> e
 * 34. <params> -> ID : <type> <other_params>
 */
bool params();

/*
 * @brief process params as arguments for function
 *
 * 35. <other_params> -> , ID : <type> <other_params>
 * 36. <other_params> -> e
 */
bool other_params();

/*
 * @brief process all possible types of variables as arguments in functions
 *
 * 37. <type_params> -> <type> <other_types_params>
 * 38. <type_params> -> e
 */
bool type_params();

/*
 * @brief process all possible args
 *
 * 39. <args> -> <param_to_func> <other_args>
 * 40. <args> -> e
 */
bool args();

/*
 * @brief process args
 *
 * 41. <param_to_func> -> ID_VAR
 * 42. <param_to_func> -> TERM
 */
bool param_to_func();

/*
 * @brief process all possible other args
 *
 * 43. <other_args> -> , <param_to_func> <other_args>
 * 44. <other_args> -> e
 */
bool other_args();

/*
 * @brief get first token and start the parser
 *
 * @return On success NO_ERR, otherwise PARSER/INTERNAL/SCANNER error
 */
int parser();

#endif // _PARSER_H
