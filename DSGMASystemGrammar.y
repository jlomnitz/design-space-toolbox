/**
* \file DSGMASystemGrammar.y
* \brief Grammar file with functions for parsing gma systems, specifying the
*        grammar used by lemon to generate the parser.
*
* \details This file specifies the generated parser by the lemon program, and 
* is the source code responsible for variable parsing.  To generate the 
* grammar implementation file, the following command must be executed:
* "lemon -m -l DSGMASystemGrammar.y ; makeheaders DSGMASystemGrammar.c".  The
* grammar for parsing GMA Systems assumes that constants cannot have exponents,
* since exponents can only be constants, and constants to a constant power is 
* also a constant.  To resolve this, any string to be parsed for a GMA System
* should be preprocessed by making it a DSExpression.
*
* Copyright (C) 2011 Jason Lomnitz.\n\n
*
* This file is part of the Design Space Toolbox V2 (C Library).
*
* The Design Space Toolbox V2 is free software: you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* The Design Space Toolbox V2 is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with the Design Space Toolbox. If not, see 
* <http://www.gnu.org/licenses/>.
*
* \author Jason Lomnitz.
* \date 2011
*/

%name DSGMASystemParser
%token_prefix  TOKEN_GMA_
%type ID {char *}

%nonassoc EQUALS.
%left PLUS MINUS.
%left DIVIDE TIMES.
%left PRIME NOT.
%right POWER.

%extra_argument {void **parser_aux}

%start_symbol start

%parse_accept {
}

%parse_failure {
        DSGMAParserAuxSetParserFailed((gma_parseraux_t *)*parser_aux);
        DSError(M_DS_PARSE ": Parsing failed", A_DS_ERROR);
}

%syntax_error {
        DSGMAParserAuxSetParserFailed((gma_parseraux_t *)*parser_aux);
        DSError(M_DS_PARSE ": Syntax error", A_DS_ERROR);
}

%include {
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "DSTypes.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSExpressionTokenizer.h"
#include "DSGMASystemParsingAux.h"
}

start ::= equation.

equation ::= expression. {
        printf("Not equations\n");
}

equation ::= ID EQUALS expression.{
        printf("Equation!\n");
}

equation ::= ID PRIME EQUALS expression.{
        printf("Equation! (Prime)\n");
}

expression ::= pterms PLUS mterms.

expression ::= mterms PLUS pterms.

expression ::= expression PLUS mterms.

expression ::= expression PLUS pterms.

pterms ::= term.{
        DSGMAParserAuxSetSign(*parser_aux, AUX_SIGN_POSITIVE);
        DSGMAParserAuxNewTerm(*parser_aux);
        *parser_aux = DSGMAParserAuxNextNode(*parser_aux);
}

pterms ::= pterms PLUS term.{
        DSGMAParserAuxSetSign(*parser_aux, AUX_SIGN_POSITIVE);
        DSGMAParserAuxNewTerm(*parser_aux);
        *parser_aux = DSGMAParserAuxNextNode(*parser_aux);
}

mterms ::= mterm. {
        DSGMAParserAuxSetSign(*parser_aux, AUX_SIGN_NEGATIVE);
        DSGMAParserAuxNewTerm(*parser_aux);
        *parser_aux = DSGMAParserAuxNextNode(*parser_aux);
}

mterms ::= mterms PLUS mterm. {
        DSGMAParserAuxSetSign(*parser_aux, AUX_SIGN_NEGATIVE);
        DSGMAParserAuxNewTerm(*parser_aux);
        *parser_aux = DSGMAParserAuxNextNode(*parser_aux);
}

mterm ::= MINUS term. [NOT]

term ::= powerlaw.

term ::= PLUS powerlaw. [NOT]

term ::= term TIMES powerlaw.

term ::= term DIVIDE powerlaw.

powerlaw ::= CONSTANT(A). {
        DSGMAParserAuxAddConstantBase(*parser_aux, DSExpressionTokenDouble((struct expression_token *)A));
}

powerlaw ::= ID(A). {
        DSGMAParserAuxAddVariableExponentPair(*parser_aux,
                                              DSExpressionTokenString((struct expression_token *)A), 1.0);
}

powerlaw ::= ID(A) POWER CONSTANT(B). {
        DSGMAParserAuxAddVariableExponentPair(*parser_aux,
        DSExpressionTokenString((struct expression_token *)A),
        DSExpressionTokenDouble((struct expression_token *)B));
}

powerlaw ::= ID(A) POWER MINUS CONSTANT(B). {
        DSGMAParserAuxAddVariableExponentPair(*parser_aux,
        DSExpressionTokenString((struct expression_token *)A),
        -DSExpressionTokenDouble((struct expression_token *)B));
}















