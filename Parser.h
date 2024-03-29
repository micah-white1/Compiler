#ifndef _Parser
#define _Parser

#include <stdio.h>
#include "SourcePosition.h"
#include "SourceFile.h"
#include "Token.h"
#include "Scanner.h"
#include "./AST/IntTypeDenoter.h"
#include "ErrorReporter.h"
#include "import_headers.h"

#include <string>
using namespace std;

// StdEnvironment* getvariables;

class Parser {

  Scanner* lexicalAnalyser;
  ErrorReporter* errorReporter;
  Token* currentToken;
  SourcePosition* previousTokenPosition;

public:
  
  Parser(Scanner* lexer, ErrorReporter* reporter);
  Program* parseProgram();
  
  void start(SourcePosition* position) ;
  void finish(SourcePosition* position);
  void acceptIt();

//Below are the methods that throw SyntaxError exception in Java
//hehe Java
  void accept (int tokenExpected);
  void syntacticError(string messageTemplate, string tokenQuoted);
  IntegerLiteral* parseIntegerLiteral();
  CharacterLiteral* parseCharacterLiteral();
  Identifier* parseIdentifier();
  Operator* parseOperator();
  Command* parseCommand();
  Command* parseSingleCommand();
  Expression* parseExpression();
  Expression* parseSecondaryExpression();
  Expression* parsePrimaryExpression();
  RecordAggregate* parseRecordAggregate();
  ArrayAggregate* parseArrayAggregate();
  Vname* parseVname ();
  Vname* parseRestOfVname(Identifier* identifierAST);
  Declaration* parseDeclaration();
  Declaration* parseSingleDeclaration();
  FormalParameterSequence* parseFormalParameterSequence();
  FormalParameterSequence* parseProperFormalParameterSequence();
  FormalParameter* parseFormalParameter();
  ActualParameterSequence* parseActualParameterSequence();
  ActualParameterSequence* parseProperActualParameterSequence();
  ActualParameter* parseActualParameter();
  TypeDenoter* parseTypeDenoter();
  FieldTypeDenoter* parseFieldTypeDenoter();
};

/*######################################################################################################################*/
/*######################################################################################################################*/
/*######################################################################################################################*/

Parser::Parser(Scanner* lexer, ErrorReporter* reporter) {
    lexicalAnalyser = lexer;
    errorReporter = reporter;
    previousTokenPosition = new SourcePosition();
	}

// accept checks whether the current token matches tokenExpected.
// If so, fetches the next token.
// If not, reports a syntactic error.

void Parser::accept (int tokenExpected) {
	if (currentToken->kind == tokenExpected) {
      previousTokenPosition = currentToken->position;
      currentToken = lexicalAnalyser->scan();
    } else {
		syntacticError("\"%\" expected here", Token::spell(tokenExpected));
    }
  }

void Parser::acceptIt() {
    previousTokenPosition = currentToken->position;
    currentToken = lexicalAnalyser->scan();
  }

// start records the position of the start of a phrase.
// This is defined to be the position of the first
// character of the first token of the phrase.

void Parser::start(SourcePosition* position) {
    position->start = currentToken->position->start;
  }

// finish records the position of the end of a phrase.
// This is defined to be the position of the last
// character of the last token of the phrase.

void Parser::finish(SourcePosition* position) {
    position->finish = previousTokenPosition->finish;
  }

void Parser::syntacticError(string messageTemplate, string tokenQuoted){
    SourcePosition* pos = currentToken->position;
	errorReporter->reportError(messageTemplate,tokenQuoted,pos);
    exit(1);
  }

///////////////////////////////////////////////////////////////////////////////
//
// PROGRAMS
//
///////////////////////////////////////////////////////////////////////////////

Program* Parser::parseProgram() {

    Program* programAST = NULL;

    previousTokenPosition->start = 0;
    previousTokenPosition->finish = 0;
    currentToken = lexicalAnalyser->scan();

    try {
      Command* cAST = parseCommand();
      programAST = new Program(cAST, previousTokenPosition);
	  if (currentToken->kind != Token::EOT) {
        syntacticError("\"%\" not expected after end of program",currentToken->spelling);
      }

    }
    catch (string s) { return NULL; }
    return programAST;
  }

///////////////////////////////////////////////////////////////////////////////
//
// LITERALS
//
///////////////////////////////////////////////////////////////////////////////

// parseIntegerLiteral parses an integer-literal, and constructs
// a leaf AST to represent it.

IntegerLiteral* Parser::parseIntegerLiteral(){
    IntegerLiteral* IL = NULL;

	if (currentToken->kind == Token::INTLITERAL) {
      previousTokenPosition = currentToken->position;
      string spelling = currentToken->spelling;
      IL = new IntegerLiteral(spelling, previousTokenPosition);
      currentToken = lexicalAnalyser->scan();
    } else {
      IL = NULL;
      syntacticError("integer literal expected here", "");
    }
    return IL;
  }

// parseCharacterLiteral parses a character-literal, and constructs a leaf
// AST to represent it.

CharacterLiteral* Parser::parseCharacterLiteral(){
    CharacterLiteral* CL = NULL;

	if (currentToken->kind == Token::CHARLITERAL) {
      previousTokenPosition = currentToken->position;
      string spelling = currentToken->spelling;
      CL = new CharacterLiteral(spelling, previousTokenPosition);
      currentToken = lexicalAnalyser->scan();
    } else {
      CL = NULL;
      syntacticError("character literal expected here", "");
    }
    return CL;
  }

// parseIdentifier parses an identifier, and constructs a leaf AST to
// represent it.

Identifier* Parser::parseIdentifier(){
    Identifier* I = NULL;
  
	if (currentToken->kind == Token::IDENTIFIER) {
      previousTokenPosition = currentToken->position;
      string spelling = currentToken->spelling;
      I = new Identifier(spelling, previousTokenPosition);
      currentToken = lexicalAnalyser->scan();
    } else {
      I = NULL;
      cout << currentToken->kind << " " << currentToken->spelling << endl;
      syntacticError("identifier expected here %",  "");
    }
    return I;
  }

// parseOperator parses an operator, and constructs a leaf AST to
// represent it.

Operator* Parser::parseOperator(){
    Operator* O = NULL;

	if (currentToken->kind == Token::OPERATOR) {
      previousTokenPosition = currentToken->position;
      string spelling = currentToken->spelling;
      O = new Operator(spelling, previousTokenPosition);
      currentToken = lexicalAnalyser->scan();
    } else {
      O = NULL;
      syntacticError("operator expected here", "");
    }
    return O;
  }

///////////////////////////////////////////////////////////////////////////////
//
// COMMANDS
//
///////////////////////////////////////////////////////////////////////////////

// parseCommand parses the command, and constructs an AST
// to represent its phrase structure.

Command* Parser::parseCommand() {
    Command* commandAST = NULL; // in case there's a syntactic error

    SourcePosition* commandPos = new SourcePosition();

    start(commandPos);
    commandAST = parseSingleCommand();
	  while (currentToken->kind == Token::SEMICOLON) {
      acceptIt();
      Command* c2AST = parseSingleCommand();
      finish(commandPos);
      commandAST = new SequentialCommand(commandAST, c2AST, commandPos);
    }
    return commandAST;
  }

Command* Parser::parseSingleCommand() {
  Command* commandAST = NULL; // in case there's a syntactic error
  SourcePosition* commandPos = new SourcePosition();
  start(commandPos);

  switch (currentToken->kind) {

	case Token::IDENTIFIER:
  {
    Identifier* iAST = parseIdentifier();
		if (currentToken->kind == Token::LPAREN) {
          acceptIt();
          ActualParameterSequence* apsAST = parseActualParameterSequence();
		      accept(Token::RPAREN);
          finish(commandPos);
          commandAST = new CallCommand(iAST, apsAST, commandPos);

        } else {

          Vname* vAST = parseRestOfVname(iAST);
		      accept(Token::BECOMES);
          Expression* eAST = parseExpression();
          finish(commandPos);
          commandAST = new AssignCommand(vAST, eAST, commandPos);
        }
  }
  break;

	case Token::BEGIN:
      acceptIt();
      commandAST = parseCommand();
	    accept(Token::END);
      break;

	case Token::LET:
      {
        acceptIt();
        Declaration* dAST = parseDeclaration();
		    accept(Token::IN_IN);
        Command* cAST = parseSingleCommand();
        finish(commandPos);
        commandAST = new LetCommand(dAST, cAST, commandPos);
      }
      break;

	case Token::IF:
      {
        acceptIt();
        Expression* eAST = parseExpression();
		    accept(Token::THEN);
        Command* c1AST = parseSingleCommand();
		    accept(Token::ELSE);
        Command* c2AST = parseSingleCommand();
        finish(commandPos);
        commandAST = new IfCommand(eAST, c1AST, c2AST, commandPos);
      }
      break;

	case Token::WHILE:
      {
        acceptIt();
        Expression* eAST = parseExpression();
		    accept(Token::DO);
        Command* cAST = parseSingleCommand();
        finish(commandPos);
        commandAST = new WhileCommand(eAST, cAST, commandPos);
      }
      break;
  case Token::REPEAT:
      {
        acceptIt();
        Command* cAST = parseSingleCommand();
        accept(Token::UNTIL);
        Expression* eAST = parseExpression();
        finish(commandPos);
        commandAST = new RepeatCommand(cAST, eAST, commandPos);
      }
      break;
  case Token::FOR:
      {
        acceptIt();
        Identifier* vAST = parseIdentifier();
        accept(Token::FROM);
        Expression* e1AST = parseExpression();
        accept(Token::TO);
        Expression* e2AST = parseExpression();
        accept(Token::DO);
        Command* cAST = parseSingleCommand();
        ConstDeclaration* dAST = new ConstDeclaration(vAST, e1AST, commandPos);
        // Declaration* dAST = new Declaration(commandPos);
        commandAST = new ForCommand(dAST, e1AST, e2AST, cAST, commandPos);
      }
      break;
  case Token::CASE:
    {
      acceptIt();
      Expression* eAST = parseExpression();
      accept(Token::OF);
      int s = 0;
      IntegerLiteral** IL = (IntegerLiteral**) malloc(1*sizeof(IntegerLiteral*));
      Command** C = (Command**) malloc((s+1)*sizeof(Command*));
      while(currentToken->kind != Token::ELSE){
        s++;

        IL = (IntegerLiteral**) realloc(IL, s*sizeof(IntegerLiteral*));
        C = (Command**) realloc(C, (s+1)*sizeof(Command*));
        
        IntegerLiteral* tempIL = parseIntegerLiteral();
        accept(Token::COLON);
        Command* tempC = parseSingleCommand();

        IL[s-1] = tempIL;
        C[s-1] = tempC;
        accept(Token::SEMICOLON);
      }
      accept(Token::ELSE);
      accept(Token::COLON);
      Command* tempC = parseSingleCommand();
      C[s] = tempC;

      commandAST = new CaseCommand(eAST, IL, C, s, commandPos);
    }
    break;
	case Token::SEMICOLON:
	case Token::END:
	case Token::ELSE:
	case Token::IN_IN:
	case Token::EOT:

      finish(commandPos);
      commandAST = new EmptyCommand(commandPos);
      break;

    default:
      syntacticError("\"%\" cannot start a command",
        currentToken->spelling);
      break;
		
  }

  return commandAST;
}

///////////////////////////////////////////////////////////////////////////////
//
// EXPRESSIONS
//
///////////////////////////////////////////////////////////////////////////////

Expression* Parser::parseExpression() {
    Expression* expressionAST = NULL; // in case there's a syntactic error

    SourcePosition* expressionPos = new SourcePosition();

    start (expressionPos);

    switch (currentToken->kind) {

	case Token::LET:
      {
        acceptIt();
        Declaration* dAST = parseDeclaration();
		    accept(Token::IN_IN);
        Expression* eAST = parseExpression();
        finish(expressionPos);
        expressionAST = new LetExpression(dAST, eAST, expressionPos);
      }
      break;

	case Token::IF:
      {
        acceptIt();
        Expression* e1AST = parseExpression();
		accept(Token::THEN);
        Expression* e2AST = parseExpression();
		accept(Token::ELSE);
        Expression* e3AST = parseExpression();
        finish(expressionPos);
        expressionAST = new IfExpression(e1AST, e2AST, e3AST, expressionPos);
      }
      break;

    default:
      expressionAST = parseSecondaryExpression();
      break;
    }
    return expressionAST;
  }

Expression* Parser::parseSecondaryExpression() {
    Expression* expressionAST = NULL; // in case there's a syntactic error

    SourcePosition* expressionPos = new SourcePosition();
    start(expressionPos);

    expressionAST = parsePrimaryExpression();
    while (currentToken->kind == Token::OPERATOR) {
        Operator* opAST = parseOperator();
        Expression* e2AST = parsePrimaryExpression();
        expressionAST = new BinaryExpression (expressionAST, opAST, e2AST,
          expressionPos);
      }
    return expressionAST;
  }

Expression* Parser::parsePrimaryExpression() {
    Expression* expressionAST = NULL; // in case there's a syntactic error

    SourcePosition* expressionPos = new SourcePosition();
    start(expressionPos);

    switch (currentToken->kind) {

	case Token::INTLITERAL:
      {
        IntegerLiteral* ilAST = parseIntegerLiteral();
        finish(expressionPos);
        expressionAST = new IntegerExpression(ilAST, expressionPos);
      }
      break;

	case Token::CHARLITERAL:
      {
        CharacterLiteral* clAST= parseCharacterLiteral();
        finish(expressionPos);
        expressionAST = new CharacterExpression(clAST, expressionPos);
      }
      break;

	case Token::LBRACKET:
      {
        acceptIt();
        ArrayAggregate* aaAST = parseArrayAggregate();
		accept(Token::RBRACKET);
        finish(expressionPos);
        expressionAST = new ArrayExpression(aaAST, expressionPos);
      }
      break;

	case Token::LCURLY:
      {
        acceptIt();
        RecordAggregate* raAST = parseRecordAggregate();
		accept(Token::RCURLY);
        finish(expressionPos);
        expressionAST = new RecordExpression(raAST, expressionPos);
      }
      break;

	case Token::IDENTIFIER:
      {
        Identifier* iAST= parseIdentifier();
		if (currentToken->kind == Token::LPAREN) {
          acceptIt();
          ActualParameterSequence* apsAST = parseActualParameterSequence();
		  accept(Token::RPAREN);
          finish(expressionPos);
          expressionAST = new CallExpression(iAST, apsAST, expressionPos);

        } else {
          Vname* vAST = parseRestOfVname(iAST);
          finish(expressionPos);
          expressionAST = new VnameExpression(vAST, expressionPos);
        }
      }
      break;

	case Token::OPERATOR:
      {
        Operator* opAST = parseOperator();
        Expression* eAST = parsePrimaryExpression();
        finish(expressionPos);
        expressionAST = new UnaryExpression(opAST, eAST, expressionPos);
      }
      break;

	case Token::LPAREN:
      acceptIt();
      expressionAST = parseExpression();
	  accept(Token::RPAREN);
      break;

    default:
      syntacticError("\"%\" cannot start an expression",
        currentToken->spelling);
      break;

    }
    return expressionAST;
  }

RecordAggregate* Parser::parseRecordAggregate() {
    RecordAggregate* aggregateAST = NULL; // in case there's a syntactic error

    SourcePosition* aggregatePos = new SourcePosition();
    start(aggregatePos);

    Identifier* iAST = parseIdentifier();
	accept(Token::IS);
    Expression* eAST = parseExpression();

	if (currentToken->kind == Token::COMMA) {
      acceptIt();
      RecordAggregate* aAST = parseRecordAggregate();
      finish(aggregatePos);
      aggregateAST = new MultipleRecordAggregate(iAST, eAST, aAST, aggregatePos);
    } else {
      finish(aggregatePos);
      aggregateAST = new SingleRecordAggregate(iAST, eAST, aggregatePos);
    }
    return aggregateAST;
  }

ArrayAggregate* Parser::parseArrayAggregate(){
    ArrayAggregate* aggregateAST = NULL; // in case there's a syntactic error

    SourcePosition* aggregatePos = new SourcePosition();
    start(aggregatePos);

    Expression* eAST = parseExpression();
	if (currentToken->kind == Token::COMMA) {
      acceptIt();
      ArrayAggregate* aAST = parseArrayAggregate();
      finish(aggregatePos);
      aggregateAST = new MultipleArrayAggregate(eAST, aAST, aggregatePos);
    } else {
      finish(aggregatePos);
      aggregateAST = new SingleArrayAggregate(eAST, aggregatePos);
    }
    return aggregateAST;
  }

///////////////////////////////////////////////////////////////////////////////
//
// VALUE-OR-VARIABLE NAMES
//
///////////////////////////////////////////////////////////////////////////////

Vname* Parser::parseVname () {
    Vname* vnameAST = NULL; // in case there's a syntactic error
    Identifier* iAST = parseIdentifier();
    vnameAST = parseRestOfVname(iAST);
    return vnameAST;
  }

Vname* Parser::parseRestOfVname(Identifier* identifierAST){
    SourcePosition* vnamePos = new SourcePosition();
    vnamePos = identifierAST->position;
    Vname* vAST = new SimpleVname(identifierAST, vnamePos);

	while (currentToken->kind == Token::DOT ||
		currentToken->kind == Token::LBRACKET) {

		if (currentToken->kind == Token::DOT) {
        acceptIt();
        Identifier* iAST = parseIdentifier();
        vAST = new DotVname(vAST, iAST, vnamePos);
      } else {
        acceptIt();
        Expression* eAST = parseExpression();
		accept(Token::RBRACKET);
        finish(vnamePos);
        vAST = new SubscriptVname(vAST, eAST, vnamePos);
      }
    }
    return vAST;
  }

///////////////////////////////////////////////////////////////////////////////
//
// DECLARATIONS
//
///////////////////////////////////////////////////////////////////////////////

Declaration* Parser::parseDeclaration() {
    Declaration* declarationAST = NULL; // in case there's a syntactic error

    SourcePosition* declarationPos = new SourcePosition();
    start(declarationPos);
    declarationAST = parseSingleDeclaration();
	while (currentToken->kind == Token::SEMICOLON) {
      acceptIt();
      Declaration* d2AST = parseSingleDeclaration();
      finish(declarationPos);
      declarationAST = new SequentialDeclaration(declarationAST, d2AST,
        declarationPos);
    }
    return declarationAST;
  }

Declaration* Parser::parseSingleDeclaration() {
    Declaration* declarationAST = NULL; // in case there's a syntactic error

    SourcePosition* declarationPos = new SourcePosition();
    start(declarationPos);

    switch (currentToken->kind) {

	case Token::CONST:
      {
        acceptIt();
        Identifier* iAST = parseIdentifier();
		    accept(Token::IS);
        Expression* eAST = parseExpression();
        finish(declarationPos);
        declarationAST = new ConstDeclaration(iAST, eAST, declarationPos);
      }
      break;

	case Token::VAR:
      {
        acceptIt();
        Identifier* iAST = parseIdentifier();
        if(currentToken->kind == Token::COLON ){
          acceptIt();
          TypeDenoter* tAST = parseTypeDenoter();
          finish(declarationPos);
          declarationAST = new VarDeclaration(iAST, tAST, declarationPos);
        }
        else{
          accept(Token::BECOMES);
          Expression* eAST = parseExpression();
          finish(declarationPos);
          declarationAST = new InitVarDeclaration(iAST, eAST, declarationPos);
        }
      }
      break;

	case Token::PROC:
      {
        acceptIt();
        Identifier* iAST = parseIdentifier();
		accept(Token::LPAREN);
        FormalParameterSequence* fpsAST = parseFormalParameterSequence();
		accept(Token::RPAREN);
		accept(Token::IS);
        Command* cAST = parseSingleCommand();
        finish(declarationPos);
        declarationAST = new ProcDeclaration(iAST, fpsAST, cAST, declarationPos);
      }
      break;

	case Token::FUNC:
      {
        acceptIt();
        if(currentToken->kind == Token::IDENTIFIER){
          Identifier* iAST = parseIdentifier();
          accept(Token::LPAREN);
          FormalParameterSequence* fpsAST = parseFormalParameterSequence();
          accept(Token::RPAREN);
          accept(Token::COLON);
          TypeDenoter* tAST = parseTypeDenoter();
          accept(Token::IS);
          Expression* eAST = parseExpression();
          finish(declarationPos);
          declarationAST = new FuncDeclaration(iAST, fpsAST, tAST, eAST, declarationPos);
        }
        else{
          Operator* oAST = parseOperator();
          accept(Token::LPAREN);
          FormalParameterSequence* fpsAST = parseFormalParameterSequence();
          accept(Token::RPAREN);
          accept(Token::COLON);
          TypeDenoter* rtAST = parseTypeDenoter();
          accept(Token::IS);
          Expression* eAST = parseExpression();
          finish(declarationPos);
          if(fpsAST->class_type() != "MULTIPLEFORMALPARAMETERSEQUENCE")
            declarationAST = new UserUnaryOperatorDeclaration(oAST, fpsAST, rtAST, eAST, declarationPos);
          else if(((MultipleFormalParameterSequence*) fpsAST)->FPS->class_type() != "MULTIPLEFORMALPARAMETERSEQUENCE"){
            declarationAST = new UserBinaryOperatorDeclaration(oAST, fpsAST, rtAST, eAST, declarationPos);
          }
          else{
            errorReporter->reportError("Operator declaration must have either 1 or 2 parameters", "", declarationPos);
          }
        }
      }
      break;

	case Token::TYPE:
      {
        acceptIt();
        Identifier* iAST = parseIdentifier();
		accept(Token::IS);
        TypeDenoter* tAST = parseTypeDenoter();
        finish(declarationPos);
        declarationAST = new TypeDeclaration(iAST, tAST, declarationPos);
      }
      break;

    default:

      syntacticError("\"%\" cannot start a declaration", currentToken->spelling);
      break;

    }
    return declarationAST;
  }

///////////////////////////////////////////////////////////////////////////////
//
// PARAMETERS
//
///////////////////////////////////////////////////////////////////////////////

FormalParameterSequence* Parser::parseFormalParameterSequence(){
    FormalParameterSequence* formalsAST;

    SourcePosition* formalsPos = new SourcePosition();

    start(formalsPos);
	if (currentToken->kind == Token::RPAREN) {
      finish(formalsPos);
      formalsAST = new EmptyFormalParameterSequence(formalsPos);

    } else {
      formalsAST = parseProperFormalParameterSequence();
    }
    return formalsAST;
  }

FormalParameterSequence* Parser::parseProperFormalParameterSequence(){
    FormalParameterSequence* formalsAST = NULL; // in case there's a syntactic error;

    SourcePosition* formalsPos = new SourcePosition();
    start(formalsPos);
    FormalParameter* fpAST = parseFormalParameter();
	if (currentToken->kind == Token::COMMA) {
      acceptIt();
      FormalParameterSequence* fpsAST = parseProperFormalParameterSequence();
      finish(formalsPos);
      formalsAST = new MultipleFormalParameterSequence(fpAST, fpsAST,
        formalsPos);

    } else {
      finish(formalsPos);
      formalsAST = new SingleFormalParameterSequence(fpAST, formalsPos);
    }
    return formalsAST;
  }

FormalParameter* Parser::parseFormalParameter(){
    FormalParameter* formalAST = NULL; // in case there's a syntactic error;

    SourcePosition* formalPos = new SourcePosition();
    start(formalPos);

  switch (currentToken->kind) {

	case Token::IDENTIFIER:
    {
      Identifier* iAST = parseIdentifier();
      accept(Token::COLON);
      TypeDenoter* tAST = parseTypeDenoter();
      finish(formalPos);
      formalAST = new ConstFormalParameter(iAST, tAST, formalPos);
    }
    break;

  case Token::IN_IN:
    {
      acceptIt();
      if(currentToken->kind == Token::OUT){
        acceptIt();
        Identifier* iAST = parseIdentifier();
        accept(Token::COLON);
        TypeDenoter* tAST = parseTypeDenoter();
        finish(formalPos);
        formalAST = new ValueResultFormalParameter(iAST, tAST, formalPos);
      }
      else{
        Identifier* iAST = parseIdentifier();
        accept(Token::COLON);
        TypeDenoter* tAST = parseTypeDenoter();
        finish(formalPos);
        formalAST = new ConstFormalParameter(iAST, tAST, formalPos);
      }
    }
    break;

  case Token::OUT:
    {
      acceptIt();
      Identifier* iAST = parseIdentifier();
      accept(Token::COLON);
      TypeDenoter* tAST = parseTypeDenoter();
      finish(formalPos);
      formalAST = new ResultFormalParameter(iAST, tAST, formalPos);
    }
    break;

	case Token::VAR:
    {
      acceptIt();
      Identifier* iAST = parseIdentifier();
      accept(Token::COLON);
      TypeDenoter* tAST = parseTypeDenoter();
      finish(formalPos);
      formalAST = new VarFormalParameter(iAST, tAST, formalPos);
    }
    break;

	case Token::PROC:
    {
      acceptIt();
      Identifier* iAST = parseIdentifier();
      accept(Token::LPAREN);
      FormalParameterSequence* fpsAST = parseFormalParameterSequence();
      accept(Token::RPAREN);
      finish(formalPos);
      formalAST = new ProcFormalParameter(iAST, fpsAST, formalPos);
    }
    break;

	case Token::FUNC:
    {
      acceptIt();
      Identifier* iAST = parseIdentifier();
      accept(Token::LPAREN);
      FormalParameterSequence* fpsAST = parseFormalParameterSequence();
      accept(Token::RPAREN);
      accept(Token::COLON);
      TypeDenoter* tAST = parseTypeDenoter();
      finish(formalPos);
      formalAST = new FuncFormalParameter(iAST, fpsAST, tAST, formalPos);
    }
    break;

  default:
    syntacticError("\"%\" cannot start a formal parameter",
      currentToken->spelling);
    break;

  }
  return formalAST;
}


ActualParameterSequence* Parser::parseActualParameterSequence(){
    ActualParameterSequence* actualsAST;

    SourcePosition* actualsPos = new SourcePosition();

    start(actualsPos);
	if (currentToken->kind == Token::RPAREN) {
      finish(actualsPos);
      actualsAST = new EmptyActualParameterSequence(actualsPos);

  } else {
    actualsAST = parseProperActualParameterSequence();
  }
    return actualsAST;
  }

ActualParameterSequence* Parser::parseProperActualParameterSequence(){
    ActualParameterSequence* actualsAST = NULL; // in case there's a syntactic error

    SourcePosition* actualsPos = new SourcePosition();

    start(actualsPos);
    ActualParameter* apAST = parseActualParameter();
	if (currentToken->kind == Token::COMMA) {
    acceptIt();
    ActualParameterSequence* apsAST = parseProperActualParameterSequence();
    finish(actualsPos);
    actualsAST = new MultipleActualParameterSequence(apAST, apsAST, actualsPos);
  } else {
    finish(actualsPos);
    actualsAST = new SingleActualParameterSequence(apAST, actualsPos);
  }
  return actualsAST;
  }

ActualParameter* Parser::parseActualParameter(){
  ActualParameter* actualAST = NULL; // in case there's a syntactic error

  SourcePosition* actualPos = new SourcePosition();

  start(actualPos);

  switch (currentToken->kind) {

	case Token::IDENTIFIER:
	case Token::INTLITERAL:
	case Token::CHARLITERAL:
	case Token::OPERATOR:
	case Token::LET:
	case Token::IF:
	case Token::LPAREN:
	case Token::LBRACKET:
	case Token::LCURLY:
    {
      Expression* eAST = parseExpression();
      finish(actualPos);
      actualAST = new ConstActualParameter(eAST, actualPos);
    }
    break;
  case Token::IN_IN:
      {
        acceptIt();
        if(currentToken->kind == Token::OUT){
          acceptIt();
          Vname* vAST = parseVname();
          finish(actualPos);
          actualAST = new ValueResultActualParameter(vAST, actualPos);
        }
        else {
          Expression* eAST = parseExpression();
          finish(actualPos);
          actualAST = new ConstActualParameter(eAST, actualPos);
        }
      }
      break;
  case Token::OUT:
      {
        acceptIt();
        Vname* vAST = parseVname();
        finish(actualPos);
        actualAST = new ResultActualParameter(vAST, actualPos);
      }
      break;
	case Token::VAR:
      {
        acceptIt();
        Vname* vAST = parseVname();
        finish(actualPos);
        actualAST = new VarActualParameter(vAST, actualPos);
      }
      break;

	case Token::PROC:
      {
        acceptIt();
        Identifier* iAST = parseIdentifier();
        finish(actualPos);
        actualAST = new ProcActualParameter(iAST, actualPos);
      }
      break;

	case Token::FUNC:
      {
        acceptIt();
        Identifier* iAST = parseIdentifier();
        finish(actualPos);
        actualAST = new FuncActualParameter(iAST, actualPos);
      }
      break;

  default:
    syntacticError("\"%\" cannot start an actual parameter", currentToken->spelling);
    break;

    }
    return actualAST;
  }

///////////////////////////////////////////////////////////////////////////////
//
// TYPE-DENOTERS
//
///////////////////////////////////////////////////////////////////////////////

TypeDenoter* Parser::parseTypeDenoter(){
    TypeDenoter* typeAST = NULL; // in case there's a syntactic error
    SourcePosition* typePos = new SourcePosition();

    start(typePos);

    switch (currentToken->kind) {

	case Token::IDENTIFIER:
      {
        Identifier* iAST = parseIdentifier();
        finish(typePos);
        typeAST = new SimpleTypeDenoter(iAST, typePos);
      }
      break;

	case Token::ARRAY:
      {
        acceptIt();
        IntegerLiteral* ilAST = parseIntegerLiteral();
		accept(Token::OF);
        TypeDenoter* tAST = parseTypeDenoter();
        finish(typePos);
        typeAST = new ArrayTypeDenoter(ilAST, tAST, typePos);
      }
      break;

	case Token::RECORD:
      {
        acceptIt();
        FieldTypeDenoter* fAST = parseFieldTypeDenoter();
		accept(Token::END);
        finish(typePos);
        typeAST = new RecordTypeDenoter(fAST, typePos);
      }
      break;

    default:
      syntacticError("\"%\" cannot start a type denoter",
        currentToken->spelling);
      break;

    }
    return typeAST;
  }

FieldTypeDenoter* Parser::parseFieldTypeDenoter() {
    FieldTypeDenoter* fieldAST = NULL; // in case there's a syntactic error

    SourcePosition* fieldPos = new SourcePosition();

    start(fieldPos);
    Identifier* iAST = parseIdentifier();
	accept(Token::COLON);
    TypeDenoter* tAST = parseTypeDenoter();
	if (currentToken->kind == Token::COMMA) {
      acceptIt();
      FieldTypeDenoter* fAST = parseFieldTypeDenoter();
      finish(fieldPos);
      fieldAST = new MultipleFieldTypeDenoter(iAST, tAST, fAST, fieldPos);
    } else {
      finish(fieldPos);
      fieldAST = new SingleFieldTypeDenoter(iAST, tAST, fieldPos);
    }
    return fieldAST;
  }
//}
#endif
