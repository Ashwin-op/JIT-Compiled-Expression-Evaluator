/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include "jitc.h"
#include "parser.h"
#include "system.h"

/* export LD_LIBRARY_PATH=. */

static const char *
generate_evaluate(const struct parser_dag *dag) {
    char *buf = malloc(1000);
    switch (dag->op) {
        case PARSER_DAG_VAL:
            sprintf(buf, "%f", dag->val);
            return buf;
        case PARSER_DAG_NEG:
            sprintf(buf, "negation(%s)", generate_evaluate(dag->right));
            return buf;
        case PARSER_DAG_ADD:
            sprintf(buf, "addition(%s, %s)", generate_evaluate(dag->left), generate_evaluate(dag->right));
            return buf;
        case PARSER_DAG_SUB:
            sprintf(buf, "subtraction(%s, %s)", generate_evaluate(dag->left), generate_evaluate(dag->right));
            return buf;
        case PARSER_DAG_MUL:
            sprintf(buf, "multiplication(%s, %s)", generate_evaluate(dag->left), generate_evaluate(dag->right));
            return buf;
        case PARSER_DAG_DIV:
            sprintf(buf, "division(%s, %s)", generate_evaluate(dag->left), generate_evaluate(dag->right));
            return buf;
        default:
            return "0";
    }
}

static void
generate(const struct parser_dag *dag, FILE *file) {
    const char *ADD = "double addition(double a, double b) { return a + b; }\n";
    const char *SUB = "double subtraction(double a, double b) { return a - b; }\n";
    const char *MUL = "double multiplication(double a, double b) { return a * b; }\n";
    const char *DIV = "double division(double a, double b) { return a / b; }\n";
    const char *NEG = "double negation(double a) { return -a; }\n";

    fprintf(file, "%s", ADD);
    fprintf(file, "%s", SUB);
    fprintf(file, "%s", MUL);
    fprintf(file, "%s", DIV);
    fprintf(file, "%s", NEG);

    fprintf(file, "double evaluate(void) { return %s; }", generate_evaluate(dag));
}

typedef double (*evaluate_t)(void);

int
main(int argc, char *argv[]) {
    const char *SOFILE = "out.so";
    const char *CFILE = "out.c";
    struct parser *parser;
    struct jitc *jitc;
    evaluate_t fnc;
    FILE *file;

    /* usage */

    if (2 != argc) {
        printf("usage: %s expression\n", argv[0]);
        return -1;
    }

    /* parse */

    if (!(parser = parser_open(argv[1]))) {
        TRACE(0);
        return -1;
    }

    /* generate C */

    if (!(file = fopen(CFILE, "w"))) {
        TRACE("fopen()");
        return -1;
    }
    generate(parser_dag(parser), file);
    parser_close(parser);
    fclose(file);

    /* JIT compile */

    if (jitc_compile(CFILE, SOFILE)) {
        file_delete(CFILE);
        TRACE(0);
        return -1;
    }
    file_delete(CFILE);

    /* dynamic load */

    if (!(jitc = jitc_open(SOFILE)) ||
        !(fnc = (evaluate_t) jitc_lookup(jitc, "evaluate"))) {
        file_delete(SOFILE);
        jitc_close(jitc);
        TRACE(0);
        return -1;
    }
    printf("%f\n", fnc());

    /* done */

    file_delete(SOFILE);
    jitc_close(jitc);
    return 0;
}
