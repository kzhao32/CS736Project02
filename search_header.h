/*
 * Copyright Kai Zhao
 * Borrowed code from Benjamin Welton's MRNet Twitter username search tutorial
 * Borrowed code from Dorian C. Arnold, Philip C. Roth, and Barton P. Miller's 
 *	MRNet IntegerAddition example
 *
 * This is search_header:
 * 	contains enums so FE knows what to pass and so BE knows what to do
 */

#include <vector>		//for std::vector
#include <string>		//for std::string

typedef enum { PROT_EXIT=FirstApplicationTag, 
               PROT_STARTPROC
             } Protocol;
