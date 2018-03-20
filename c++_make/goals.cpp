#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "make.h"

extern Goals goals;

Goal::Goal()
{
	name = NULL;
	subgoal = (char **) malloc(sizeof(char *));
	subgoal[0] = NULL;
	command = (char **) malloc(sizeof(char *));
	command[0] = NULL;
}

Goal::~Goal()
{
	delete [] name;
	
	for(int i = 0; subgoal[i] != NULL; i++)
		delete [] subgoal[i];
	free(subgoal);

	for(int i = 0; command[i] != NULL; i++)
		delete [] command[i];
	free(command);
}

void Goal::set_name(const char *n)
{
	if(name != NULL)
		delete [] name;

	name = new char[strlen(n) + 1];
	strcpy(name, n);
}
void Goal::add_subgoal(const char *sg)
{
	int i;
	for(i = 0; subgoal[i] != NULL; i++);
	subgoal = (char **) realloc(subgoal, (i + 2) * sizeof(char *));
	subgoal[i] = new char[strlen(sg) + 1];
	strcpy(subgoal[i], sg);
	subgoal[i + 1] = NULL;
}

void Goal::add_command(const char *c)
{
	int i;
	for(i = 0; command[i] != NULL; i++);
	command = (char **) realloc(command, (i + 2) * sizeof(char *));
	command[i] = new char[strlen(c) + 1];
	strcpy(command[i], c);
	command[i + 1] = NULL;
}


void Goal::add_to_command(const char *c)
{
	int i;
	for(i = 0; command[i] != NULL; i++);
	i--;
	char *buf = new char[strlen(command[i]) + strlen(c) + 1];
	strcpy(buf, command[i]);
	strcat(buf, c);
	delete [] command[i];
	command[i] = buf;
}

Goals::Goals()
{
	goal = NULL;
	last = -1;
}

Goals::~Goals()
{
	for(int i = 0; i <= last; i++)
		goal[i].~Goal();

	free(goal);
}

void Goals::add_goal()
{
	last++;
	goal = (Goal *) realloc(goal, (last + 1) * sizeof(Goal));
	goal[last].name = NULL;
	goal[last].subgoal = (char **) malloc(sizeof(char *));
	goal[last].subgoal[0] = NULL;
	goal[last].command = (char **) malloc(sizeof(char *));
	goal[last].command[0] = NULL;
}

Var::Var()
{
	name = NULL;
	value = (Lexeme *) malloc(sizeof(Lexeme));
	value[0].set_type(END);
	value[0].set_coord(0, 0, 0, 0);
	value[0].lex_string = NULL;
}

Var::~Var()
{
	if(name != NULL)
		delete [] name;

	for(int i = 0; value[i].type() != END; i++)
		value[i].~Lexeme();

	free(value);

	name = NULL;
	value = NULL;
}

Var & Var::operator =(Var & new_var)
{
	if(name != NULL)
		delete [] name;

	if(value != NULL)
	{
		for(int i = 0; value[i].type() != END; i++)
			value[i].~Lexeme();
	
		free(value);
	}	
	name = NULL;
	value = NULL;

	if(new_var.name != NULL)
	{
		name = new char[strlen(new_var.name) + 1];
		strcpy(name, new_var.name);
	}
	
	if(new_var.value != NULL)	
	{	
		int i;
		for(i = 0; new_var.value[i].type() != END; i++);
	
		value = (Lexeme *) malloc((i + 1) * sizeof(Lexeme));
	
		for(int j = 0; j <= i; j++)
		{
			value[j].lex_string = NULL;
			value[j] = new_var.value[j];
		}
	}
	return *this;
}

void Var::set_name(const char *s)
{
	if(name != NULL)
		delete [] name;

	name = new char[strlen(s) + 1];
	strcpy(name, s);
}

void Var::add_value(Lexeme & lex)
{
	int i;
	for(i = 0; value[i].type() != END; i++);
	
	value = (Lexeme *) realloc(value, (i + 2) * sizeof(Lexeme));
	value[i + 1].lex_string = NULL;
	value[i + 1] = value[i];
	value[i] = lex;
}

void Var::del()
{
	for(int i = 0; value[i].type() != END; i++)
		value[i].~Lexeme();

	free(value);

	value = (Lexeme *) malloc(sizeof(Lexeme));
	value[0].set_type(END);
	value[0].set_coord(0, 0, 0, 0);
	value[0].lex_string = NULL;
}

Vars::Vars()
{
	var = (Var *) malloc(sizeof(Var));

	var[0].name = NULL;
	var[0].value = NULL;
}

Vars::~Vars()
{
	for(int i = 0; var[i].name != NULL; i++)
		var[i].~Var();

	free(var);
}

int Vars::find(const char *vname)
{
	int i;
	
	for(i = 0; var[i].name != NULL; i++)
		if(strcmp(var[i].name, vname) == 0)
			return i;
	return -1;
}

void Vars::add(Var & new_var)
{
	int i;
	for(i = 0; var[i].name != NULL; i++)
		if(strcmp(var[i].name, new_var.name) == 0)
			break;

	if(var[i].name == NULL)
	{	
		var = (Var *) realloc(var, (i + 2) * sizeof(Var));
		var[i + 1].name = NULL;
		var[i + 1].value = NULL;
		var[i + 1] = var[i];
	}
	
	var[i] = new_var;
}

Files::Files()
{
	path = (char **) malloc(sizeof(char *));
	file = (FILE **) malloc(sizeof(FILE *));
	c_buf = (int *) malloc(sizeof(int));

	path[0] = NULL;
	file[0] = NULL;
	c_buf[0] = '\0';
	last = -1;
}

Files::~Files()
{
	for(int i = 0; path[i] != NULL; i++)
		delete [] path[i];
	
	free(path);

	for(int i = 0; file[i] != NULL; i++)
		fclose(file[i]);

	free(file);

	free(c_buf);
}

int Files::add_file(const char * fpath, int c)
{
	if(fpath == NULL)
		return -1;
	
	FILE *f = fopen(fpath, "r");
	
	if(f == NULL)
	{
		fprintf(stderr, "Failed to open %s\n", fpath);
		return -2;
	}
	int i;
	for(i = 0; path[i] != NULL; i++)
		if(strcmp(path[i], fpath) == 0)
		{
			fprintf(stderr, "Multiple include of file %s\n", fpath);
			fclose(f);
			return -3;
		}
	
	last++;

	path = (char **) realloc(path, (i + 2) * sizeof(char *));
	path[i] = new char[strlen(fpath) + 1];
	strcpy(path[i], fpath);
	path[i + 1] = NULL;

	for(i = 0; file[i] != NULL; i++);

	file = (FILE **) realloc(file, (i + 2) * sizeof(FILE *));
	file[i] = f;
	file[i + 1] = NULL;

	c_buf = (int *) realloc(c_buf, (i + 2) * sizeof(int));
	c_buf[i] = c;
	c_buf[i + 1] = '\0';

	return 0;
}

int Files::close_one()
{
	int c;
	int i;
	for(i = 0; file[i] != NULL; i++);
	
	fclose(file[i - 1]);
	file[i - 1] = NULL;
	c = c_buf[i - 1];
	c_buf[i - 1] = '\0';

	last--;

	file = (FILE **) realloc(file, i * sizeof(FILE *));
	c_buf = (int *) realloc(c_buf, i * sizeof(int));
	
	return c;
}


Tree::Tree()
{
	construct();
}

Tree::~Tree()
{
	for(int i = 0; child[i] != NULL; i++)
		delete child[i];
	
	free(child);
}

void Tree::construct()
{
	value = -1;
	parent = NULL;
	child = (Tree **) malloc(sizeof(Tree *));
	child[0] = NULL;
}

void Tree::add_child(int val)
{
	int i;
	for(i = 0; child[i] != NULL; i++);

	child = (Tree **) realloc(child, (i + 2) * sizeof(Tree *));
	child[i] = new Tree;
	child[i]->parent = this;
	child[i]->value = val;
	child[i + 1] = NULL;
}

void Tree::del_child()
{
	int i;
	for(i = 0; child[i] != NULL; i++);
	
	if(i == 0)
		return;
	
	delete child[i - 1];
	child[i - 1] = NULL;
	child = (Tree **) realloc(child, i * sizeof(Tree *));
}

void Tree::print(int nest)
{
	if(value >= 0)
	{
		for(int i = 4; i < nest; i++)
			putchar(' ');

		printf("%s\n", goals.goal[value].name);
	}

	for(int i = 0; child[i] != NULL; i++)
	{
		child[i]->print(nest + 4);
	}
}


Array::Array(int s)
{
	size = s;
	if(size > 0)
	{
		value = (bool *) malloc(size * sizeof(bool));
		reset();
	}
	else
		value = NULL;
}

Array::~Array()
{
	free(value);
}

void Array::add(bool v)
{
	size++;
	value = (bool *) realloc(value, size * sizeof(bool));
	value[size - 1] = v;
}

void Array::reset()
{
	for(int i = 0;i < size; i++)
		value[i] = false;
}
