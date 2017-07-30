/*
 * File:   debug.h
 * Author: Pedro Sánchez Alguacil
 *
 * Created on 19 de marzo de 2015, 12:06
 */

#ifndef DEBUG_H
#define	DEBUG_H

/**
 * @brief Debug flag.
 **/
#ifndef DEBUG
#define DEBUG 0
#endif

//#define debug_print(...) \
//si, ahi estoy            do { if (DEBUG>=1) fprintf(stderr, ##__VA_ARGS__); } while (0)

#define debug_print_2(...) \
            do { if (DEBUG>=2) fprintf(stderr, ##__VA_ARGS__); } while (0)

#endif	/* DEBUG_H */

