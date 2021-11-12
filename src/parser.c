/**
 * Project: Compiler IFJ21
 *
 * @file parser.c
 *
 * @brief Implement functions processing rules for syntax analysis
 *
 * @author Andrei Shchapaniak <xshcha00>
 */

#include <stdio.h>
#include <stdlib.h>
#include "expressions.h"
#include "parser.h"
#include "error.h"
#include "str.h"

token_t token;
int err;
static bool ret;
htab_t *h_table;

bool prog() {
    if (token.keyword == KW_REQUIRE) {
        print_rule("1.  <prog> -> <prolog> <prog>");

        NEXT_NONTERM(prolog);
        return prog();
    }
    else if (token.keyword == KW_GLOBAL) {
        print_rule("2.  <prog> -> global id : function ( <type_params> ) <type_returns> <prog>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_ID);

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_COLON);

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.keyword == KW_FUNCTION);

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_L_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(type_params);

        EXPECTED_TOKEN(token.type == T_R_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(type_returns);

        return prog();
    }
    else if (token.keyword == KW_FUNCTION) {
        print_rule("3.  <prog> -> function id ( <params> ) <type_returns> <statement>"
                " <return_type> end <prog>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_ID);

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_L_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(params);

        EXPECTED_TOKEN(token.type == T_R_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(type_returns);

        NEXT_NONTERM(statement);

        EXPECTED_TOKEN(token.keyword == KW_END);

        NEXT_TOKEN();
        return prog();
    }
    else if (token.type == T_ID) {
        print_rule("4.  <prog> -> id_func ( <args> ) <prog>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_L_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(args);

        EXPECTED_TOKEN(token.type == T_R_ROUND_BR);

        NEXT_TOKEN();
        return prog();
    }
    else if (token.type == T_EOF) {
        print_rule("5.  <prog> -> EOF");

        return true;
    }

    return false;
}

bool prolog() {
    print_rule("6.  <prolog> -> require term_str");

    EXPECTED_TOKEN(token.keyword == KW_REQUIRE);

    NEXT_TOKEN();
    EXPECTED_TOKEN(!str_cmp_const_str(&token.attr.id, "ifj21") && token.type == T_STRING);

    NEXT_TOKEN();
    return true;
}

bool type() {
    if (token.type == T_KEYWORD) {
        if (token.keyword == KW_INTEGER) {
            print_rule("7.  <type> -> integer");
        }
        else if (token.keyword == KW_NUMBER) {
            print_rule("8.  <type> -> number");
        }
        else if (token.keyword == KW_STRING) {
            print_rule("9.  <type> -> string");
        }
        else if (token.keyword == KW_NIL) {
            print_rule("10. <type> -> nil");
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    NEXT_TOKEN();
    return true;
}

bool statement() {
    if (token.keyword == KW_IF) {
        print_rule("11. <statement> -> if <expression> then <statement> else <statement>"
                " end <statement>");

        NEXT_TOKEN();
        NEXT_NONTERM(expression);

        EXPECTED_TOKEN(token.keyword == KW_THEN);

        NEXT_TOKEN();
        NEXT_NONTERM(statement);

        EXPECTED_TOKEN(token.keyword == KW_ELSE);

        NEXT_TOKEN();
        NEXT_NONTERM(statement);

        EXPECTED_TOKEN(token.keyword == KW_END);

        NEXT_TOKEN();
        return statement();
    }
    else if (token.keyword == KW_WHILE) {
        print_rule("12. <statement> -> while <expression> do <statement>"
                " end <statement>");

        NEXT_TOKEN();
        NEXT_NONTERM(expression);

        EXPECTED_TOKEN(token.keyword == KW_DO);

        NEXT_TOKEN();
        NEXT_NONTERM(statement);

        EXPECTED_TOKEN(token.keyword == KW_END);

        NEXT_TOKEN();
        return statement();
    }
    else if (token.keyword == KW_LOCAL) {
        print_rule("13. <statement> -> local id_var : <type> <def_var> <statement>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_ID);

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_COLON);

        NEXT_TOKEN();
        NEXT_NONTERM(type);

        NEXT_NONTERM(def_var);

        return statement();
    }
    else if (token.keyword == KW_RETURN) {
        print_rule("15. <statement> -> return <expression> <other_exp> <statement>");

        NEXT_TOKEN();
        NEXT_NONTERM(expression);

        NEXT_NONTERM(other_exp);

        return statement();
    }
    else if (token.type == T_ID) {
        print_rule("14. <statement> -> id <work_var> <statement>");

        NEXT_TOKEN();
        NEXT_NONTERM(work_var);

        return statement();
    }

    print_rule("16. <statement> -> e");
    return true;
}

bool work_var() {
    if (token.type == T_L_ROUND_BR) {
        print_rule("17. <work_var> -> ( <args> )");

        NEXT_TOKEN();
        NEXT_NONTERM(args);

        EXPECTED_TOKEN(token.type == T_R_ROUND_BR);

        NEXT_TOKEN();
    }
    else {
        print_rule("18. <work_var> -> <vars>");
        return vars();
    }

    return true;
}

bool vars() {
    if (token.type == T_COMMA) {
        print_rule("19. <vars> -> , id_var <vars>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_ID);

        NEXT_TOKEN();
        return vars();
    }
    else if (token.type == T_ASSIGN) {
        print_rule("20. <vars> -> = <type_expr>");

        NEXT_TOKEN();
        return type_expr();
    }

    return false;
}

bool type_expr() {
    if (token.type == T_ID) {
        print_rule("21. <type_expr> -> id_func ( <args> )");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_L_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(args);

        EXPECTED_TOKEN(token.type == T_R_ROUND_BR);

        NEXT_TOKEN();
        return true;
    }

    print_rule("22. <type_expr> -> <expression> <other_exp>");

    NEXT_NONTERM(expression);
    return other_exp();
}

bool other_exp() {
    if (token.type == T_COMMA) {
        print_rule("23. <other_exp> -> , <expression> <other_exp>");

        NEXT_TOKEN();
        NEXT_NONTERM(expression);

        return other_exp();
    }

    print_rule("24. <other_exp> -> e");
    return true;
}

bool def_var() {
    if (token.type == T_ASSIGN) {
        print_rule("25. <def_var> -> = <init_assign>");

        NEXT_TOKEN();
        return init_assign();
    }

    print_rule("26. <def_var> -> e");
    return true;
}

bool init_assign() {
    if (token.type == T_ID) {
        print_rule("27. <init_assign> -> id_func ( <args> )");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_L_ROUND_BR);

        NEXT_TOKEN();
        NEXT_NONTERM(args);

        EXPECTED_TOKEN(token.type == T_R_ROUND_BR);

        NEXT_TOKEN();
        return true;
    }

    print_rule("28. <init_assign> -> <expression>");
    return expression();
}

bool type_returns() {
    if (token.type == T_COLON) {
        print_rule("29. <type_returns> -> : <type> <other_types>");

        NEXT_TOKEN();
        NEXT_NONTERM(type);

        return other_types();
    }

    print_rule("30. <type_returns> -> e");
    return true;
}

bool other_types() {
    if (token.type == T_COMMA) {
        print_rule("31. <other_types> -> , <type> <other_types>");

        NEXT_TOKEN();
        NEXT_NONTERM(type);

        return other_types();
    }

    print_rule("32. <other_types> -> e");
    return true;
}

bool params() {
    if (token.type == T_ID) {
        print_rule("34. <params> -> id : <type> <other_params>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_COLON);

        NEXT_TOKEN();
        NEXT_NONTERM(type);

        return other_params();
    }

    print_rule("33. <params> -> e");
    return true;
}

bool other_params() {
    if (token.type == T_COMMA) {
        print_rule("35. <other_params> -> , id : <type> <other_params>");

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_ID);

        NEXT_TOKEN();
        EXPECTED_TOKEN(token.type == T_COLON);

        NEXT_TOKEN();
        NEXT_NONTERM(type);

        return other_params();
    }

    print_rule("36. <other_params> -> e");
    return true;
}

bool type_params() {
    if (type()) {
        print_rule("37. <type_params> -> <type> <other_types>");
        return other_types();
    }

    print_rule("38. <type_params> -> e");
    return true;
}

bool args() {
    if (TOKEN_ID_TERM())
    {
        print_rule("39. <args> -> id_var <other_args>");

        NEXT_TOKEN();
        return other_args();
    }

    print_rule("40. <args> -> e");
    return true;
}

bool other_args() {
    if (token.type == T_COMMA) {
        print_rule("41. <other_args> -> , <arg> <other_args>");

        NEXT_TOKEN();

        if (TOKEN_ID_TERM())
        {
            NEXT_TOKEN();
            return other_args();
        }

        return false;
    }

    print_rule("42. <other_args> -> e");
    return true;
}

int parser() {
    FILE *f = stdin;
    err = NO_ERR;
    set_source_file(f);

    ret = str_init(&token.attr.id, 20);
    if (!ret) {
        return INTERNAL_ERR;
    }

    h_table = symtab_init();
    if (!ret) {
        str_free(&token.attr.id);
        return INTERNAL_ERR;
    }

    FIRST_TOKEN();
    ret = prog();

    if (!ret && err == NO_ERR) {
        err = PARSER_ERR;
    }

    str_free(&token.attr.id);
    symtab_free(h_table);

    return err;
}