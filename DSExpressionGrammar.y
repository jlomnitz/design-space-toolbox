//  
/**
* \file DSExpressionGrammar.y
* \brief Grammar file with functions for parsing variables, specifying the
*        grammar used by lemon to generate the parser.
*
* \details This file specifies the generated parser by the lemon program, and 
* is the source code responsible for variable parsing.  To generate the 
* grammar implementation file, the following command must be executed:
* "lemon -q -m -l DSExpressionGrammar.y ; makeheaders DSExpressionGrammar.c".
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

%name DSExpressionParser
%token_prefix  TOKEN_EXPRESSION_
%token_type {DSExpression *}
%type ID {void *}
%destructor expr {DSExpressionFree($$);}

%nonassoc EQUALS.
%left PLUS MINUS.
%left DIVIDE TIMES.
%left PRIME NOT.
%right POWER.

%extra_argument {void *parsed}

%start_symbol program

%parse_accept {
}

%parse_failure {
        DSError(M_DS_PARSE ": Parsing failed", A_DS_ERROR);
        ((parse_expression_s *)parsed)->wasSuccesful = false;
}

%syntax_error {
        DSError(M_DS_PARSE ": Syntax error", A_DS_ERROR);
        ((parse_expression_s *)parsed)->wasSuccesful = false;
}

%include {
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "DSTypes.h"
#include "DSExpression.h"
#include "DSExpressionTokenizer.h"

extern DSExpression * dsExpressionAllocWithOperator(const char op_code);
extern DSExpression * dsExpressionAllocWithConstant(const double value);
extern DSExpression * dsExpressionAllocWithVariableName(const char * name);

}

program ::= expr(B). {
        if (parsed == NULL) {
                DSError(M_DS_WRONG ": parser structure is NULL", A_DS_ERROR);
                DSExpressionFree(B);
        } else {
                ((parse_expression_s *)parsed)->root = B;
        }
}

program ::= equation(B). {
        if (parsed == NULL) {
                DSError(M_DS_WRONG ": parser structure is NULL", A_DS_ERROR);
                DSExpressionFree(B);
        } else {
                ((parse_expression_s *)parsed)->root = B;
        }
}

equation(A) ::= expr(B) EQUALS expr(C). {
        A = dsExpressionAllocWithOperator('=');
        DSExpressionAddBranch(A, B);
        DSExpressionAddBranch(A, C);
}

expr(A) ::= expr(B) PLUS expr(C). {
        if (DSExpressionType(B) == DS_EXPRESSION_TYPE_CONSTANT &&
                DSExpressionType(B) == DSExpressionType(C)) {
                A = dsExpressionAllocWithConstant(DSExpressionConstant(B)+DSExpressionConstant(C));
                DSExpressionFree(B);
                DSExpressionFree(C);
        } else {
                A = dsExpressionAllocWithOperator('+');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        }
}

expr(A) ::= expr(B) MINUS expr(C). {
        if (DSExpressionType(B) == DS_EXPRESSION_TYPE_CONSTANT &&
                DSExpressionType(B) == DSExpressionType(C)) {
                A = dsExpressionAllocWithConstant(DSExpressionConstant(B)-DSExpressionConstant(C));
                DSExpressionFree(B);
                DSExpressionFree(C);
        } else if (DSExpressionType(C) == DS_EXPRESSION_TYPE_CONSTANT) {
                A = dsExpressionAllocWithConstant(-DSExpressionConstant(C));
                DSExpressionFree(C);
                C = A;
                A = dsExpressionAllocWithOperator('+');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        } else {
                A = dsExpressionAllocWithOperator('*');
                DSExpressionAddBranch(A, C);
                DSExpressionAddBranch(A, dsExpressionAllocWithConstant(-1.0));
                C = A;
                A = dsExpressionAllocWithOperator('+');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        }
}

expr(A) ::= expr(B) TIMES expr(C). {
        if (DSExpressionType(B) == DS_EXPRESSION_TYPE_CONSTANT &&
                DSExpressionType(B) == DSExpressionType(C)) {
                A = dsExpressionAllocWithConstant(DSExpressionConstant(B)*DSExpressionConstant(C));
                DSExpressionFree(B);
                DSExpressionFree(C);
        } else {
                A = dsExpressionAllocWithOperator('*');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        }
}

expr(A) ::= expr(B) DIVIDE expr(C). {
        if (DSExpressionType(B) == DS_EXPRESSION_TYPE_CONSTANT &&
            DSExpressionType(B) == DSExpressionType(C)) {
                A = dsExpressionAllocWithConstant(DSExpressionConstant(B)/DSExpressionConstant(C));
                DSExpressionFree(B);
                DSExpressionFree(C);
        } else if (DSExpressionType(C) == DS_EXPRESSION_TYPE_CONSTANT) {
                A = dsExpressionAllocWithConstant(pow(DSExpressionConstant(C), -1.0));
                DSExpressionFree(C);
                C = A;
                A = dsExpressionAllocWithOperator('*');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        } else {
                A = dsExpressionAllocWithOperator('^');
                DSExpressionAddBranch(A, C);
                DSExpressionAddBranch(A, dsExpressionAllocWithConstant(-1.0));
                C = A;
                A = dsExpressionAllocWithOperator('*');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        }
}

expr(A) ::= expr(B) POWER expr(C). {
        if (DSExpressionType(B) == DS_EXPRESSION_TYPE_CONSTANT &&
            DSExpressionType(C) == DS_EXPRESSION_TYPE_CONSTANT) {
                A = dsExpressionAllocWithConstant(pow(DSExpressionConstant(B), DSExpressionConstant(C)));
                DSExpressionFree(B);
                DSExpressionFree(C);
        } else {
                A = dsExpressionAllocWithOperator('^');
                DSExpressionAddBranch(A, B);
                DSExpressionAddBranch(A, C);
        }
}

expr(A) ::= MINUS expr(B). [NOT] {
        if (DSExpressionType(B) == DS_EXPRESSION_TYPE_CONSTANT) {
                A = dsExpressionAllocWithConstant(-DSExpressionConstant(B));
                DSExpressionFree(B);
        } else {
                A = dsExpressionAllocWithOperator('*');
                DSExpressionAddBranch(A, dsExpressionAllocWithConstant(-1.0));
                DSExpressionAddBranch(A, B);
        }
}

expr(A) ::= PLUS expr(B). [NOT] {
        A = B;
}

expr(A) ::= expr(B) PRIME. {
        A = dsExpressionAllocWithOperator('.');
        DSExpressionAddBranch(A, B);
}

expr(A) ::= ID(B). {
        A = dsExpressionAllocWithVariableName(DSExpressionTokenString((struct expression_token *)B));
}

expr(A) ::= VALUE(B). {
        A = dsExpressionAllocWithConstant(DSExpressionTokenDouble((struct expression_token *)B));
}

expr(A) ::= ID(B) LPAREN expr(C) RPAREN. {
        A = dsExpressionAllocWithVariableName(DSExpressionTokenString((struct expression_token *)B));
        DSExpressionAddBranch(A, C);
}

expr(A) ::= LPAREN expr(B) RPAREN. {
        A = B;
}



