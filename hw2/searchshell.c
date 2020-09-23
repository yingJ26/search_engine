/*
 * Copyright ©2020 Travis McGaha.  All rights reslisterved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Summer Quarter 2020 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * intereslistted in reusing these course materials should contact the
 * author.
 */

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"
#include "FileParser.h"

#define BUFFSIZE 500

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
static void Usage(void);
// search inside MemIndex
// print out the found file name from the DocTable
static void ProcessQueries(DocTable *dt, MemIndex *mi);

//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        Usage();
    }
    if (argc == 3) {
        if (strcmp(argv[2], "-s") != 0) {
            Usage();
        } else {
            SetFilter(true);
        }
    }
    // Implement searchshell!  We're giving you very few hints
    // on how to do it, so you'll need to figure out an appropriate
    // decomposition into functions as well as implementing the
    // functions.  There are several major tasks you need to build:
    //
    //  - Crawl from a directory provided by argv[1] to produce and index
    //  - Prompt the user for a query and read the query from stdin, in a loop
    //  - Split a query into words (check out strtok_r)
    //  - Process a query against the index and print out the reslistults
    //
    // When searchshell detects end-of-file on stdin (cntrl-D from the
    // keyboard), searchshell should free all dynamically allocated
    // memory and any other allocated reslistources and then exit.
    DocTable* dt = DocTable_Allocate();
    MemIndex* mi = MemIndex_Allocate();
    printf("Indexing \'%s\'\n", argv[1]);
    if (!CrawlFileTree(argv[1], &dt, &mi)) {
        Usage();
        return EXIT_FAILURE;
    }
    ProcessQueries(dt, mi);
    printf("Shut down...\n");
    MemIndex_Free(mi);
    DocTable_Free(dt);
    return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
    fprintf(stderr, "Usage: ./searchshell <docroot>\n");
    fprintf(stderr,
            "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
    exit(EXIT_FAILURE);
}

static void ProcessQueries(DocTable *dt, MemIndex *mi) {
    char *token;
    char buf[BUFFSIZE];
    char* query[BUFFSIZE];
    char *saveptr;
    int i = 0;

    while (1) {
        printf("enter query:\n");
        // make sure to exit when CTRL+D
        if (fgets(buf, BUFFSIZE, stdin) == NULL) {
            break;
        } else {
            i = 0;
            // covert the get chars into lower case
            while (buf[i]) {
                char a = buf[i];
                buf[i] = tolower(a);
                i++;
            }
            i = 0;
            char* tobeParsed = buf;
            // fgets will include \n, replace the last char by \0
            buf[strlen(buf)-1] = '\0';
            // extract tokens from the string, store them separatly inside query
            token = strtok_r(tobeParsed, " ", &saveptr);
            do {
                query[i] = token;
                i++;
                tobeParsed = saveptr;
                token = strtok_r(tobeParsed, " ", &saveptr);
            } while (token != NULL);

            LinkedList* reslist = LinkedList_Allocate();
            // process the query against the MemIndex
            reslist = MemIndex_Search(mi, query, i);
            // iterate the reslist and print out SearchResult's filename
            if (reslist != NULL && LinkedList_NumElements(reslist) != 0) {
                LLIterator* iter = LLIterator_Allocate(reslist);
                bool next = true;
                while (LLIterator_IsValid(iter) && next) {
                    SearchResult *search;
                    LLIterator_Get(iter, (void *)&search);
                    char* filename = DocTable_GetDocName(dt, search->docid);
                    printf("  %s (%d)\n", filename, search->rank);
                    next = LLIterator_Next(iter);
                }
                LLIterator_Free(iter);
                LinkedList_Free(reslist, &free);
            }
        }
    }
}


