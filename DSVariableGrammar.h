/* This file was automatically generated.  Do not edit! */
#define DSVariablePoolParserTOKENTYPE double *
#define DSVariablePoolParserARG_PDECL ,DSVariablePool *pool
void DSVariablePoolParser(void *yyp,int yymajor,DSVariablePoolParserTOKENTYPE yyminor DSVariablePoolParserARG_PDECL);
#if defined(YYTRACKMAXSTACKDEPTH)
int DSVariablePoolParserStackPeak(void *p);
#endif
void DSVariablePoolParserFree(void *p,void(*freeProc)(void *));
void *DSVariablePoolParserAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void DSVariablePoolParserTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define DSVariablePoolParserARG_STORE yypParser->pool = pool
#define DSVariablePoolParserARG_FETCH DSVariablePool *pool = yypParser->pool
#define DSVariablePoolParserARG_SDECL DSVariablePool *pool;
#define TOKEN_ASSIGN                          6
#define TOKEN_SEPERATOR                       5
#define TOKEN_QUOTE                           4
#define TOKEN_OTHER                           3
#define TOKEN_IDENTIFIER                      2
#define TOKEN_VALUE                           1
#define INTERFACE 0
