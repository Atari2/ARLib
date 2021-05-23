#pragma once

#define STRINGIFY(TOK) #TOK
#define TOSTRING(X) STRINGIFY(X)
#define CONCAT_IMPL(A, B) A ## B
#define CONCAT_NL_IMPL(A, B) A ## "\n" ## B
#define CONCAT(A, B) CONCAT_IMPL(A, B)
#define CONCAT_NL(A, B) CONCAT_NL_IMPL(A, B)

#define LINEINFO CONCAT("At line ", TOSTRING(__LINE__))
#define FILEINFO CONCAT("In file ", __FILE__)

#define ERRINFO CONCAT_NL(LINEINFO, FILEINFO)

#ifdef DEBUG
#define DEBUGMSG(msg) puts(msg);
#define DEBUGFMTMSG(msg, ...) printf(msg, __VA_ARGS__);
#else
#define DEBUGMSG(msg) (void)0;
#define DEBUGFMTMSG(msg, ...) (void)0;
#endif