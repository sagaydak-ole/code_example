#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "make.h"

Lexeme::Lexeme()
{
	lex_type = 0;
	lex_coordinates[0] = 0;
	lex_coordinates[1] = 0;
	lex_coordinates[2] = 0;
	lex_coordinates[3] = 0;
	lex_string = NULL;
}

Lexeme::~Lexeme()
{
	if(lex_string != NULL)
		delete [] lex_string;
}

void Lexeme::set_type(int t)
{
	lex_type = t;
}

void Lexeme::set_string(const char *str)
{
	if(lex_string != NULL)
		delete [] lex_string;

	if(str == NULL)
		lex_string = NULL;
	else
	{
		lex_string = new char[strlen(str) + 1];
		strcpy(lex_string, str);
	}
}

void Lexeme::set_coord(int c1, int c2, int c3, int c4)
{
	lex_coordinates[0] = c1;
	lex_coordinates[1] = c2;
	lex_coordinates[2] = c3;
	lex_coordinates[3] = c4;
}

Lexeme & Lexeme::operator =(Lexeme &copy)
{
	set_type(copy.lex_type);
	set_coord(copy.lex_coordinates[0], copy.lex_coordinates[1], copy.lex_coordinates[2], copy.lex_coordinates[3]);
	set_string(copy.lex_string);
	return *this;
}

int Lexeme::type(void)
{
	return lex_type;
}

char* Lexeme::string(void)
{
	return lex_string;
}

int* Lexeme::coord(void)
{
	return lex_coordinates;
}

void Lexeme::print()
{
	const char *tstr;
	switch(lex_type)
	{
		case END:			tstr = "END"; break;
		case NAME:			tstr = "NAME"; break;
		case TAB:			tstr = "TAB"; break;
		case WHITESPACE:	tstr = "WHITESPACE"; break;
		case SEMICOLON:		tstr = "SEMICOLON"; break;
		case RIGHT_PART:	tstr = "RIGHT_PART"; break;
		case LEFT_PART:		tstr = "LEFT_PART"; break;
		case INCLUDE:		tstr = "INCLUDE"; break;
		case PHONY:			tstr = "PHONY"; break;
		case EQUALS:		tstr = "EQUALS"; break;
		case VAR_CALL:		tstr = "VAR_CALL"; break;
		case NEWLINE:		tstr = "NEWLINE"; break;
		default:			tstr = "UNKNOWN"; break;
	}
	printf("%10s | (%2d,%2d,%2d,%2d) | %s\n", tstr, lex_coordinates[0], lex_coordinates[1], lex_coordinates[2], lex_coordinates[3], lex_string);
}

/**/

Lex_str::Lex_str()
{
	str = (char*) malloc(sizeof(char));
	str[0] = '\0';
}

Lex_str::~Lex_str()
{
	free(str);
}

void Lex_str::add(int c)
{
	int len = strlen(str);
	str = (char*) realloc(str, (len + 2) * sizeof(char));
	str[len] = c;
	str[len + 1] = '\0';
}

void Lex_str::del()
{
	str = (char*) realloc(str, sizeof(char));
	str[0] = '\0';
}
