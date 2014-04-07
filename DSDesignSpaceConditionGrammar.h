/* This file was automatically generated.  Do not edit! */
#define DSDesignSpaceConstraintParserTOKENTYPE void*
#define DSDesignSpaceConstraintParserARG_PDECL ,void **parser_aux
void DSDesignSpaceConstraintParser(void *yyp,int yymajor,DSDesignSpaceConstraintParserTOKENTYPE yyminor DSDesignSpaceConstraintParserARG_PDECL);
#if defined(YYTRACKMAXSTACKDEPTH)
int DSDesignSpaceConstraintParserStackPeak(void *p);
#endif
void DSDesignSpaceConstraintParserFree(void *p,void(*freeProc)(void *));
void *DSDesignSpaceConstraintParserAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void DSDesignSpaceConstraintParserTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define DSDesignSpaceConstraintParserARG_STORE yypParser->parser_aux = parser_aux
#define DSDesignSpaceConstraintParserARG_FETCH void **parser_aux = yypParser->parser_aux
#define DSDesignSpaceConstraintParserARG_SDECL void **parser_aux;
#define TOKEN_DSC_POWER                          12
#define TOKEN_DSC_NOT                            11
#define TOKEN_DSC_PRIME                          10
#define TOKEN_DSC_TIMES                           9
#define TOKEN_DSC_DIVIDE                          8
#define TOKEN_DSC_MINUS                           7
#define TOKEN_DSC_PLUS                            6
#define TOKEN_DSC_MT                              5
#define TOKEN_DSC_LT                              4
#define TOKEN_DSC_EQUALS                          3
#define TOKEN_DSC_CONSTANT                        2
#define TOKEN_DSC_ID                              1
#define INTERFACE 0
