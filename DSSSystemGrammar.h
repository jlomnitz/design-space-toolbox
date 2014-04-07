/* This file was automatically generated.  Do not edit! */
#define DSSSystemParserTOKENTYPE void*
#define DSSSystemParserARG_PDECL ,void **parser_aux
void DSSSystemParser(void *yyp,int yymajor,DSSSystemParserTOKENTYPE yyminor DSSSystemParserARG_PDECL);
#if defined(YYTRACKMAXSTACKDEPTH)
int DSSSystemParserStackPeak(void *p);
#endif
void DSSSystemParserFree(void *p,void(*freeProc)(void *));
void *DSSSystemParserAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void DSSSystemParserTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define DSSSystemParserARG_STORE yypParser->parser_aux = parser_aux
#define DSSSystemParserARG_FETCH void **parser_aux = yypParser->parser_aux
#define DSSSystemParserARG_SDECL void **parser_aux;
#define TOKEN_SSYS_POWER                          12
#define TOKEN_SSYS_NOT                            11
#define TOKEN_SSYS_PRIME                          10
#define TOKEN_SSYS_TIMES                           9
#define TOKEN_SSYS_DIVIDE                          8
#define TOKEN_SSYS_MINUS                           7
#define TOKEN_SSYS_PLUS                            6
#define TOKEN_SSYS_MT                              5
#define TOKEN_SSYS_LT                              4
#define TOKEN_SSYS_EQUALS                          3
#define TOKEN_SSYS_CONSTANT                        2
#define TOKEN_SSYS_ID                              1
#define INTERFACE 0
