/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * jitc.c
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include "system.h"
#include "jitc.h"

/**
 * Needs:
 *   fork()
 *   execv()
 *   waitpid()
 *   WIFEXITED()
 *   WEXITSTATUS()
 *   dlopen()
 *   dlclose()
 *   dlsym()
 */

/* research the above Needed API and design accordingly */

struct jitc {
    void *handle;
};

/**
 * Compiles a C program into a dynamically loadable module.
 *
 * input : the file pathname of the C program
 * output: the file pathname of the dynamically loadable module
 *
 * return: 0 on success, otherwise error
 */

int
jitc_compile(const char *input, const char *output) {
    pid_t pid;
    int status;
    char *argv[7];

    argv[0] = "gcc";
    argv[1] = "-shared";
    argv[2] = "-o";
    argv[3] = (char *) output;
    argv[4] = "-fPIC";
    argv[5] = (char *) input;
    argv[6] = NULL;

    if ((pid = fork()) < 0) {
        TRACE("fork");
        return -1;
    }

    if (0 == pid) {
        execv("/usr/bin/gcc", argv);
        TRACE("execv");
        return -1;
    }

    if (pid != waitpid(pid, &status, 0)) {
        TRACE("waitpid");
        return -1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
        TRACE("WIFEXITED");
        return -1;
    }

    return 0;
}

/**
 * Loads a dynamically loadable module into the calling process' memory for
 * execution.
 *
 * pathname: the file pathname of the dynamically loadable module
 *
 * return: an opaque handle or NULL on error
 */

struct jitc *
jitc_open(const char *pathname) {
    struct jitc *jitc;

    if (!(jitc = malloc(sizeof(*jitc)))) {
        TRACE("malloc");
        return NULL;
    }

    if (!(jitc->handle = dlopen(pathname, RTLD_LAZY))) {
        TRACE(dlerror());
        free(jitc);
        return NULL;
    }

    return jitc;
}

/**
 * Unloads a previously loaded dynamically loadable module.
 *
 * jitc: an opaque handle previously obtained by calling jitc_open()
 *
 * Note: jitc may be NULL
 */

void
jitc_close(struct jitc *jitc) {
    if (!jitc) {
        return;
    }

    if (dlclose(jitc->handle)) {
        TRACE(dlerror());
    }

    free(jitc);
}

/**
 * Searches for a symbol in the dynamically loaded module associated with jitc.
 *
 * jitc: an opaque handle previously obtained by calling jitc_open()
 *
 * return: the memory address of the start of the symbol, or 0 on error
 */

long
jitc_lookup(struct jitc *jitc, const char *symbol) {
    void *address;

    if (!jitc) {
        return 0;
    }

    if (!(address = dlsym(jitc->handle, symbol))) {
        TRACE(dlerror());
        return 0;
    }

    return (long) address;
}
