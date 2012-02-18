//  
/**
 * \file DSVariableGrammar.y
 * \brief Grammar file with functions for parsing variables, specifying the
 *        grammar used by lemon to generate the parser.
 *
 * \details This file specifies the generated parser by the lemon program, and 
 * is the source code responsible for variable parsing.  To generate the 
 * grammar implementation file, the following command must be executed:
 * "lemon -m -q -l DSVariableGrammar.y ; makeheaders DSVariableGrammar.c".
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
%name DSVariablePoolParser
%token_prefix  TOKEN_
%type VALUE {double *}
%type IDENTIFIER {char *}
%token_type {double *}

%extra_argument {DSVariablePool *pool}

%nonassoc OTHER.
%nonassoc QUOTE.
%nonassoc SEPERATOR.
%nonassoc ASSIGN.
%nonassoc IDENTIFIER.
%nonassoc VALUE.

%start_symbol program

%parse_accept {
        
}

%parse_failure {
        DSError(M_DS_PARSE ": Parsing failed", A_DS_ERROR);
}

%syntax_error {
        DSError(M_DS_PARSE ": Syntax error", A_DS_WARN);
}

%include {
#include <string.h>
#include <assert.h>
#include "DSTypes.h"
#include "DSVariable.h"
#include "DSVariableTokenizer.h"
}

program ::= expr.

expr ::= statement.

expr ::= expr SEPERATOR statement.

statement ::= QUOTE IDENTIFIER(A) QUOTE ASSIGN VALUE(B). {
        DSVariablePoolAddVariableWithName(pool, (const char *)A);
        DSVariablePoolSetValueForVariableWithName(pool, (const char *)A, *B);
}

statement ::= IDENTIFIER(A) ASSIGN VALUE(B). {
        DSVariablePoolAddVariableWithName(pool, (const char *)A);
        DSVariablePoolSetValueForVariableWithName(pool, (const char *)A, *B);
}

statement ::= IDENTIFIER(A). {
        DSVariablePoolAddVariableWithName(pool, (const char *)A);
}



