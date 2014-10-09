/* This file was automatically generated.  Do not edit! */
#define DSCaseOptimizationFunctionParserTOKENTYPE void*
#define DSCaseOptimizationFunctionParserARG_PDECL ,void **parser_aux
void DSCaseOptimizationFunctionParser(void *yyp,int yymajor,DSCaseOptimizationFunctionParserTOKENTYPE yyminor DSCaseOptimizationFunctionParserARG_PDECL);
#if defined(YYTRACKMAXSTACKDEPTH)
int DSCaseOptimizationFunctionParserStackPeak(void *p);
#endif
void DSCaseOptimizationFunctionParserFree(void *p,void(*freeProc)(void *));
void *DSCaseOptimizationFunctionParserAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void DSCaseOptimizationFunctionParserTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define DSCaseOptimizationFunctionParserARG_STORE yypParser->parser_aux = parser_aux
#define DSCaseOptimizationFunctionParserARG_FETCH void **parser_aux = yypParser->parser_aux
#define DSCaseOptimizationFunctionParserARG_SDECL void **parser_aux;
#define TOKEN_COF_POWER                          12
#define TOKEN_COF_NOT                            11
#define TOKEN_COF_PRIME                          10
#define TOKEN_COF_TIMES                           9
#define TOKEN_COF_DIVIDE                          8
#define TOKEN_COF_MINUS                           7
#define TOKEN_COF_PLUS                            6
#define TOKEN_COF_MT                              5
#define TOKEN_COF_LT                              4
#define TOKEN_COF_EQUALS                          3
#define TOKEN_COF_CONSTANT                        2
#define TOKEN_COF_ID                              1
#define INTERFACE 0
