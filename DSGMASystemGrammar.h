/* This file was automatically generated.  Do not edit! */
#define DSGMASystemParserTOKENTYPE void*
#define DSGMASystemParserARG_PDECL ,void **parser_aux
void DSGMASystemParser(void *yyp,int yymajor,DSGMASystemParserTOKENTYPE yyminor DSGMASystemParserARG_PDECL);
#if defined(YYTRACKMAXSTACKDEPTH)
int DSGMASystemParserStackPeak(void *p);
#endif
void DSGMASystemParserFree(void *p,void(*freeProc)(void *));
void *DSGMASystemParserAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void DSGMASystemParserTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define DSGMASystemParserARG_STORE yypParser->parser_aux = parser_aux
#define DSGMASystemParserARG_FETCH void **parser_aux = yypParser->parser_aux
#define DSGMASystemParserARG_SDECL void **parser_aux;
#define TOKEN_GMA_POWER                          12
#define TOKEN_GMA_NOT                            11
#define TOKEN_GMA_PRIME                          10
#define TOKEN_GMA_TIMES                           9
#define TOKEN_GMA_DIVIDE                          8
#define TOKEN_GMA_MINUS                           7
#define TOKEN_GMA_PLUS                            6
#define TOKEN_GMA_MT                              5
#define TOKEN_GMA_LT                              4
#define TOKEN_GMA_EQUALS                          3
#define TOKEN_GMA_CONSTANT                        2
#define TOKEN_GMA_ID                              1
#define INTERFACE 0
