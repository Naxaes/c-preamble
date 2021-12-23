#ifndef PREAMBLE_HEADER_INCLUDE_GUARD
#define PREAMBLE_HEADER_INCLUDE_GUARD

#include <stdint.h>  /* The sized integer types */
#include <stddef.h>  /* size_t */

/* Some sources
https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
https://dev.to/rdentato/
*/

/* ---- TYPES ----
Concise and specific sizes, as well as some meaningful names
for certain types like Unicode and booleans.
*/
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float  f32;
typedef double f64;

typedef size_t usize;

typedef u32  rune;  /* Unicode code point   */
typedef char utf8;  /* Unicode byte         */


#ifndef __cplusplus
typedef i8 bool;
#ifndef true
#define true  1
#endif
#ifndef false
#define false 0
#endif
#endif


/* ---- THE THREE RULES OF MACROS ----
1. A variable must never be referenced more than once.
2. A variable must always be put inside of parenthesis.
3. A rule must always be followed unless you don't feel like it.
*/


/* ---- MACRO UTILITIES ----  */
#define ARRAY_COUNT(x)     (sizeof(x) / sizeof(*(x)))

/* NOTE(ted): Needed to evaluate other macros.  */
#define INTERNAL_CONCAT_HELP(x, y)  x ## y
#define INTERNAL_CONCATENATE(x, y)  INTERNAL_CONCAT_HELP(x, y)
#ifndef __COUNTER__
/* NOTE(ted): Can cause conflicts if used multiple times at the same line, even
    if the lines are in different files.
*/
#define UNIQUE_NAME(name)  INTERNAL_CONCATENATE(name ## _, __LINE__)
#else
#define UNIQUE_NAME(name)  INTERNAL_CONCATENATE(name ## _, __COUNTER__)
#endif


/* ---- DEBUGGING, LOGGING AND ASSERTIONS ----
NOTE(ted): Do I need to flush before DEBUG_BREAK,  as the interrupt might
interrupt the output?
*/
#ifndef ERROR_LOGGER
#include <stdio.h>  /* fprintf, stderr */
#define ERROR_LOGGER(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef STANDARD_LOGGER
#include <stdio.h>  /* printf */
#define STANDARD_LOGGER(...) printf(__VA_ARGS__)
#endif

#ifndef DEBUG_BREAK
#include <signal.h>  /* raise, SIGABRT. SIGTRAP. SIGINT */
/* TODO(ted): What am I'm doing... Fix this "raise EVERYTHING!!". */
#define DEBUG_BREAK() (raise(SIGABRT)/*, raise(SIGTRAP), raise(SIGINT)*/)
#endif

#define HEADER(group)      STANDARD_LOGGER("%s:%d [" #group "]: ", __FILE__, __LINE__)
#define ERR_HEADER(group)  ERROR_LOGGER("%s:%d [" #group "]: ", __FILE__, __LINE__)

#define LOG(string)       (HEADER(LOG), STANDARD_LOGGER(string "\n"))
#define LOGF(format, ...) (HEADER(LOG), STANDARD_LOGGER(format "\n", __VA_ARGS__))

#define ERROR(group, string)       (ERR_HEADER(group), ERROR_LOGGER(string "\n"), fflush(stderr), DEBUG_BREAK())
#define ERRORF(group, format, ...) (ERR_HEADER(group), ERROR_LOGGER(format "\n", __VA_ARGS__), DEBUG_BREAK())

#define PANIC(string)       ERROR(PANIC, string)
#define PANICF(format, ...) ERRORF(PANIC, format, __VA_ARGS__)

#define ASSERT(x)       ((x) ? 0 : (ERROR(ASSERT, "'" #x "' is false.\n")))
#define ASSERTF(x, ...) ((x) ? 0 : (ERR_HEADER(ASSERT), ERROR_LOGGER("'" #x "' is false. "),  ERROR_LOGGER(__VA_ARGS__), ERROR_LOGGER("\n"), DEBUG_BREAK()))

#define STATIC_ASSERT(x) extern int UNIQUE_NAME(STATIC_ASSERTION)[(x) ? 1 : -1]

#define NO_DEFAULT      default: ERROR(NO_DEFAULT, "Default case was unexpectedly hit.")
#define INVALID_PATH    ERROR(INVALID_PATH, "Invalid path.")
#define NOT_IMPLEMENTED ERROR(NOT_IMPLEMENTED, "Not implemented.")


/* ---- BIT MANIPULATION ---- */
#define BIT_SET(x, bit)    ((x) |=   (1ULL << (bit)))
#define BIT_CLEAR(x, bit)  ((x) &=  ~(1ULL << (bit)))
#define BIT_FLIP(x, bit)   ((x) ^=   (1ULL << (bit)))
#define BIT_CHECK(x, bit)  (!!((x) & (1ULL << (bit))))

#define BITMASK_SET(x, mask)        ((x) |=   (mask))
#define BITMASK_CLEAR(x, mask)      ((x) &= (~(mask)))
#define BITMASK_FLIP(x, mask)       ((x) ^=   (mask))
#define BITMASK_CHECK_ALL(x, mask)  (!(~(x) & (mask)))
#define BITMASK_CHECK_ANY(x, mask)  ((x) &    (mask))


/* ---- ENUMS ----
Generate enums and matching strings by writing a macro that takes a parameter F,
and then define your enums by writing F(<name>).

    // Define
    #define FOR_EACH_MY_ENUM(F) \
        F(MY_ENUM_1) \
        F(MY_ENUM_2) \
        ...

    // Declare
    typedef enum { FOR_EACH_MY_ENUM(GENERATE_ENUM) } MyThing;
    static String MY_ENUM_STRINGS[] = { FOR_EACH_MY_ENUM(GENERATE_STRING) };

You can also declare your own macro `F` for other types of generation. For
example:

    // Generate a new type for each enum.
    #define GENERATE_STRUCTS(name) typedef struct name { } name;
    // Generate global constants for your enums.
    #define GENERATE_CONSTANTS(name) const u8 MY_THING_ ## name = name;
*/
#define GENERATE_ENUM(name) name,
#define GENERATE_STRING(name) (String) { #name , sizeof(#name) },


/* ---- DEFAULT ARGUMENTS ----
Wrap the function in a variadic macro that calls to WITH_DEFAULTS with a
name and __VA_ARGS__. For each parameter passed in it'll now dispatch
to macros (or functions) named "nameX", where X is the number of parameters.

    // Define the dispatcher.
    #define greet(...) WITH_DEFAULTS(greet, __VA_ARGS__)

    // Define the overloaded functions (or function-like macros) you want.
    #define greet1(name)           printf("%s %s!", "Hello", name)
    #define greet2(greeting, name) printf("%s %s!", greeting, name)

    // Call.
    greet("Sailor");                      // printf("%s %s!", "Hello",     "Sailor");
    greet("Greetings", "Sailor");         // printf("%s %s!", "Greetings", "Sailor");
    greet("Greetings", "Sailor", "!!!");  // Error: greet3 is not defined.

This is restricted to a minimum of 1 argument and a maximum of 8.
*/
#define POP_10TH_ARG(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, argN, ...) argN
#define VA_ARGS_COUNT(...)  POP_10TH_ARG(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define SELECT_FUNCTION(function, postfix) CONCATENATE(function, postfix)
#define WITH_DEFAULTS(f, ...) SELECT_FUNCTION(f, VA_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)


/* ---- FORMATTING ----
`printf` doesn't support printing in binary. These macros can be used to
accomplish that.

    u64 x = ...;
    printf(BINARY_FORMAT_PATTERN_64 "\n", BYTE_TO_BINARY_CHARS_64(x));
*/
#ifndef BINARY_FORMAT_PATTERN_PREFIX
#define BINARY_FORMAT_PATTERN_PREFIX "0b"
#endif
#ifndef BINARY_FORMAT_PATTERN_DELIMITER
#define BINARY_FORMAT_PATTERN_DELIMITER "_"
#endif

#define BINARY_FORMAT_PATTERN_8  BINARY_FORMAT_PATTERN_PREFIX "%c%c%c%c%c%c%c%c"
#define BINARY_FORMAT_PATTERN_16 BINARY_FORMAT_PATTERN_PREFIX "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c"
#define BINARY_FORMAT_PATTERN_32 BINARY_FORMAT_PATTERN_PREFIX "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c"
#define BINARY_FORMAT_PATTERN_64 BINARY_FORMAT_PATTERN_PREFIX "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c" BINARY_FORMAT_PATTERN_DELIMITER "%c%c%c%c%c%c%c%c"

#define BYTE_TO_BINARY_CHARS_8(x)  BIT_CHECK((u8)(x), 7) ? '1' : '0', BIT_CHECK((u8)(x), 6) ? '1' : '0', BIT_CHECK((u8)(x), 5) ? '1' : '0', BIT_CHECK((u8)(x), 4) ? '1' : '0', BIT_CHECK((u8)(x), 3) ? '1' : '0', BIT_CHECK((u8)(x), 2) ? '1' : '0', BIT_CHECK((u8)(x), 1) ? '1' : '0',  BIT_CHECK((u8)(x), 0) ? '1' : '0'
#define BYTE_TO_BINARY_CHARS_16(x) BYTE_TO_BINARY_CHARS_8(((u16)  (x)) >> 8),  BYTE_TO_BINARY_CHARS_8(((u16)  (x)))
#define BYTE_TO_BINARY_CHARS_32(x) BYTE_TO_BINARY_CHARS_16(((u32) (x)) >> 16), BYTE_TO_BINARY_CHARS_16(((u32) (x)))
#define BYTE_TO_BINARY_CHARS_64(x) BYTE_TO_BINARY_CHARS_32(((u64) (x)) >> 32), BYTE_TO_BINARY_CHARS_32(((u64) (x)))



/* ---- OS DETECTION ---- */
#ifdef _WIN32
#define OS_NAME "Windows 32-bit"
#define OS_IS_WINDOWS_32 1
#endif
#if _WIN64
#define OS_NAME "Windows 64-bit"
#define OS_IS_WINDOWS_64 1
#endif
#if __CYGWIN__  /* Windows with Cygwin (POSIX) */
#define OS_NAME "Windows 32-bit (Cygwin)"
#define OS_IS_WINDOWS_CYGWIN 1
#endif
#if __APPLE__ || __MACH__
#define OS_NAME "Mac OSX"
#define OS_IS_MAC_OSX 1
#endif
#if __linux__  /* any GNU/Linux distribution */
#define OS_NAME "Linux"
#define OS_IS_LINUX 1
#endif
#if __unix || __unix__
#define OS_NAME "Unix"
#define OS_IS_UNIX 1
#endif
#if __FreeBSD__
#define OS_NAME "FreeBSD"
#define OS_IS_FREE_BSD 1
#endif
#if defined(BSD)  /* BSD (DragonFly BSD, FreeBSD, OpenBSD, NetBSD)  */
#define OS_NAME "BSD (DragonFly BSD, FreeBSD, OpenBSD, NetBSD)"
#define OS_IS_BSD 1
#endif
#if defined(__QNX__)
#define OS_NAME "QNX"
#define OS_IS_QNX 1
#endif
#if _AIX
#define OS_NAME "AIX"
#define OS_IS_AIX 1
#endif
#if __hpux
#define OS_NAME "HP-UX"
#define OS_IS_HP_UX 1
#endif
#if __sun  /* Solaris */
#define OS_NAME "Solaris"
#define OS_IS_SOLARIS 1
#endif


#endif  /* PREAMBLE_HEADER_INCLUDE_GUARD */