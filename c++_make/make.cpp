#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include "make.h"

void Start(void);
void Name(void);
void Subgoals(void);
void Var_init(void);
void Phony(void);
void Include(void);
void Include_file(void);
void Command_tab(void);
void Command(void);
void Error(void);

int Build_Tree(Tree *);
void Execute_Tree(Tree *);

int gc(void);
void getlex(void);

Lexeme lex;
Lex_str buf;
bool slash = false;
int tempc;
int found;

int c;

bool istemplate = false;
bool isphony = false;

Array met_in_branch(0);
Array met_in_tree(0);

Files files;

Goals goals;
Goals templates;
Goal phony;
Tree root;

Vars vars;
Var left;
Var right;
Var var_buf;

int row = 0;
int col	= 0;
int col1 = 0;
int row_b = 0;
int col_b = 0;
int n = 1;
int lex_regime = 0;

int Argc;
char **Argv;

extern char *optarg;

int main(int argc, char *argv[])
{
	int opt;
	char *file = new char[strlen("Mymakefile") + 1];
	strcpy(file, "Mymakefile");
	if((opt = getopt(argc, argv, "f:")) >= 0)
	{
		if(opt == 'f')
		{
			n = 3;
			delete [] file;
			file = new char[strlen(optarg) + 1];
			strcpy(file, optarg);
		}
		else
		{
			delete [] file;
			fprintf(stderr, "Incorrect input\n");
			return 1;
		}
	}

	
	lex.set_type(0);
	lex.set_coord(0, 0, 0, 0);
	lex.set_string(NULL);
	
	if(files.add_file(file, 0) < 0)
	{
		delete [] file;
		return 2;
	}
	delete [] file;

	Argc = argc;
	Argv = argv;
	
	c = gc();

	getlex();
	
	while(!feof(files.file[0]) && lex.type() != END)
	{
	
	//	getlex();
	//	lex.print();
		
		
		Start();
	}
	
	if(lex.type() != END)
	{
		/*

		for(int i = 0; phony.subgoal[i] != NULL; i++)
			printf("'%s' is PHONY\n", phony.subgoal[i]);

		for(int i = 0; i <= goals.last; i++)
		{
			printf("||%s||:", goals.goal[i].name);
			
			for(int j = 0; goals.goal[i].subgoal[j] != NULL; j++)
				printf(" |%s|", goals.goal[i].subgoal[j]);
			
			printf("\n");
		
			for(int j = 0; goals.goal[i].command[j] != NULL; j++)
				printf("\t>>%s\n", goals.goal[i].command[j]);
		}
		
		printf("\n\n");

		for(int i = 0; i <= templates.last; i++)
		{
			printf("||%s||:", templates.goal[i].name);
			
			for(int j = 0; templates.goal[i].subgoal[j] != NULL; j++)
				printf(" |%s|", templates.goal[i].subgoal[j]);
			
			printf("\n");
		
			for(int j = 0; templates.goal[i].command[j] != NULL; j++)
				printf("\t>>%s\n", templates.goal[i].command[j]);
		}
		*/
		
		for(int i = 0; i <= goals.last; i++)
		{
			met_in_tree.add(false);
			met_in_branch.add(false);
		}

		if(Build_Tree(&root) == 0)
		{
//			printf("\n\n");
//			root.print(0);
//			printf("\n\n");
			Execute_Tree(&root);
		}
	}
	return 0;
}

// FIRST PART

void getlex()
{
	static int status = START;
	
	static int lexi = 0;
	if(lex_regime == -2)
	{
		if(left.value[lexi].type() != END)
		{
			lex = left.value[lexi];
			lexi++;
			return;
		}

		lexi = 0;
		lex_regime = 0;
	}

	if(lex_regime == -1)
	{
		if(right.value[lexi].type() != END)
		{
			lex = right.value[lexi];
			lexi++;
			return;
		}

		lexi = 0;
		lex_regime = 0;
	}

	if(lex_regime > 0)
	{
		if(vars.var[lex_regime - 1].value[lexi].type() != END)
		{
			lex = vars.var[lex_regime - 1].value[lexi];
			lexi++;
			return;
		}

		lexi = 0;
		lex_regime = 0;
	}

	do
	{
		switch(status)
		{
			case START:
				switch(c)
				{
					case '\t':
						status = READ;
						lex.set_type(TAB);
						lex.set_coord(row, col - 1, row, col - 1);
						lex.set_string("\t");
						c = gc();
						return;
						break;

					case ' ':
						status = WHITESPACE;
						row_b = row;
						col_b = col - 1;
						buf.add(c);
						c = gc();
						break;

					case '\n':
						lex.set_type(NEWLINE);
						lex.set_coord(row - 1, col1, row - 1, col1);
						lex.set_string(NULL);
						c = gc();
						return;
						break;

					case '$':
						status = DOLLAR;
						row_b = row;
						col_b = col - 1;
						c = gc();
						break;

					case ':':
						status = READ;
						lex.set_type(SEMICOLON);
						lex.set_coord(row, col - 1, row, col - 1);
						lex.set_string(":");
						c = gc();
						return;
						break;

					case '=':
						status = READ;
						lex.set_type(EQUALS);
						lex.set_coord(row, col - 1, row, col - 1);
						lex.set_string("=");
						c = gc();
						return;
						break;

					case '#':
						status = COMMENT;
						c = gc();
						break;

					case '\\':
						c = gc();
						if(c != '\n')
						{
							status = NAME;
							row_b = row;
							col_b = col - 2;
							buf.add('\\');
						}
						else
							c = gc();
						break;
					
					default:
						status = NAME;
						row_b = row;
						col_b = col - 1;
						buf.add(c);
						c = gc();
						break;
				}
				break;

			case NAME:
				switch(c)
				{
					case '\n': case ' ': case '\t': case ':': case '=': case '$':
						status = READ;
						if(!strcmp(buf.str, "include"))
							lex.set_type(INCLUDE);
						else if(!strcmp(buf.str, ".PHONY"))
							lex.set_type(PHONY);
						else
							lex.set_type(NAME);

						lex.set_string(buf.str);
						lex.set_coord(row_b, col_b, (col == 0) ? (row - 1) : row, ((col == 0) ? col1 : (col - 1)) - (col > 1));
						buf.del();
						return;
						break;

					case '\\':
						c = gc();
						if(c != '\n')	
							buf.add('\\');
						else
							c = gc();
						break;

					default:
						buf.add(c);
						c = gc();
						break;
				}
				break;

			case READ:
				switch(c)
				{
					case ' ': case '\t':
						status = WHITESPACE;
						row_b = row;
						col_b = col - 1;
						buf.add(c);
						c = gc();
						break;

					case '\n':
						status = START;
						lex.set_type(NEWLINE);
						lex.set_coord(row - 1, col1, row - 1, col1);
						lex.set_string(NULL);
						c = gc();
						return;
						break;
					
					case '$':
						status = DOLLAR;
						row_b = row;
						col_b = col - 1;
						c = gc();
						break;

					case ':':
						lex.set_type(SEMICOLON);
						lex.set_coord(row, col - 1, row, col - 1);
						lex.set_string(":");
						c = gc();
						return;
						break;

					case '=':
						lex.set_type(EQUALS);
						lex.set_coord(row, col - 1, row, col - 1);
						lex.set_string("=");
						c = gc();
						return;
						break;

					case '\\':
						if(slash)
						{
							c = tempc;
							slash = false;
						}
						else
							c = gc();
						
						if(c != '\n')
						{
							status = NAME;
							row_b = row;
							col_b = col - 2;
							buf.add('\\');
						}
						else 
							c = gc();
						break;

					default:
						status = NAME;
						row_b = row;
						col_b = col - 1;
						buf.add(c);
						c = gc();
						break;
				}
				break;

			case WHITESPACE:
				switch(c)
				{
					case ' ': case '\t':
						buf.add(c);
						c = gc();
						break;

					case '\\':
						c = gc();
						if(c != '\n')
						{
							status = READ;
							lex.set_type(WHITESPACE);
							lex.set_string(buf.str);
							lex.set_coord(row_b, col_b, (col == 0) ? (row - 1) : row, ((col == 0) ? col1 : (col - 1)) - 2);
							buf.del();
							slash = true;
							tempc = c;
							c = '\\';
							return;
						}
						else
							c = gc();
						break;

					default:
						status = READ;
						lex.set_type(WHITESPACE);
						lex.set_string(buf.str);
						lex.set_coord(row_b, col_b, (col == 0) ? (row - 1) : row, ((col == 0) ? col1 : (col - 1)) - 1);
						buf.del();
						return;
						break;
				}
				break;

			case DOLLAR:
				switch(c)
				{
					case '@':
						c = gc();
//						if(c != ' ' && c != '\t' && c != '\n')
//						{
//							fprintf(stderr, "Invalid syntax after '$' (Position: (%d,%d))\n", row_b, col_b);
//							lex.set_type(END);
//							return;
//						}
						status = READ;
						lex.set_type(LEFT_PART);
						lex.set_string(NULL);
						lex.set_coord(row_b, col_b, (col == 0) ? (row - 1) : row, ((col == 0) ? col1 : (col - 1)) - 1);
						return;
						break;

					case '^':
						c = gc();
//						if(c != ' ' && c != '\t' && c != '\n')
//						{
//							fprintf(stderr, "Invalid syntax after '$' (Position: (%d,%d))\n", row_b, col_b);
//							lex.set_type(END);
//							return;
//						}
						status = READ;
						lex.set_type(RIGHT_PART);
						lex.set_string(NULL);
						lex.set_coord(row_b, col_b, (col == 0) ? (row - 1) : row, ((col == 0) ? col1 : (col - 1)) - 1);
						return;
						break;

					case '(':
						status = VAR;
						c = gc();
						if(!isalpha(c) && c != '_')
						{
							fprintf(stderr, "Invalid syntax after '$' (Position: (%d,%d))\n", row_b, col_b);
							lex.set_type(END);
							return;
						}
						break;

					default:
						fprintf(stderr, "Invalid syntax after '$' (Position: (%d,%d))\n", row_b, col_b);
						lex.set_type(END);
						return;
						break;
				}
				break;

			case VAR:
				switch(c)
				{
					case ')':
						c = gc();
						status = READ;
						lex.set_type(VAR_CALL);
						lex.set_string(buf.str);
						lex.set_coord(row_b, col_b, (col == 0) ? (row - 1) : row, ((col == 0) ? col1 : (col - 1)) - 1);
						buf.del();
						return;
						break;

					default:
						if(!isalpha(c) && !isdigit(c) && c != '_')
						{
							fprintf(stderr, "Invalid syntax after '$' (Position: (%d,%d))\n", row_b, col_b);	
							lex.set_type(END);
							return;
						}
						buf.add(c);
						c = gc();
						break;
				}
				break;

			case COMMENT:
				switch(c)
				{
					case '\n':
						status = START;
						// falls through
					default:
						c = gc();
						break;
				}
				break;

			default:
				printf("Unknown status\n");
				return;
				break;
		}
	}while(1);
}

int gc(void)
{
	static bool end = false;
	int c1;

	if(end)
		return '\n';

	int c = fgetc(files.file[files.last]);
	col++;
	if(c == '\n')
	{
		col1 = col - 1;
		col = 0;
		row++;
	}
	if(c == EOF)
	{
		if(files.last > 0)
		{
			c1 = files.close_one();
			
			while(c1 == EOF && files.last > 0)
			{
				c1 = files.close_one();
			}
			if(c1 == EOF)
			{
				end = true;
				return '\n';
			}
			return c1;
		}
		end = true;
		return '\n';
	}
	return c;
}

// SECOND PART

void Start()
{
	switch(lex.type())
	{
		case NAME:
			Name();
			break;

		case PHONY:
			Phony();
			break;

		case INCLUDE:
			Include();
			break;
		
		case TAB: case WHITESPACE: case NEWLINE:
			getlex();
			Start();
			break;

		case VAR_CALL:
			found = vars.find(lex.string());
			
			if(found < 0)
			{
				fprintf(stderr,"No such variable at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
				Error();
			}
			else
			{
				lex_regime = found + 1;
				getlex();
				Start();
			}
			
			break;

		default:
			fprintf(stderr,"Invalid lexeme type after start at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;
	}
}

void Name()
{
	static Lexeme lex_buf;
	
	static char * buf;
	if(lex.type() == NAME)
	{
		buf = new char[strlen(lex.string()) + 1];
		strcpy(buf, lex.string());
		lex_buf = lex;
	}
	
	getlex();
	
	switch(lex.type())
	{
		case WHITESPACE:
			Name();
			break;

		case SEMICOLON:
			if(strchr(buf, '%') != NULL)
			{
				if(strchr(buf, '%') == strrchr(buf, '%'))
				{
					for(int i = 0; i <= templates.last; i++)
					{
						if(strcmp(buf, templates.goal[i].name) == 0)
						{
							fprintf(stderr,"Multiple definition of rule \"%s\"\n", buf);
							delete [] buf;
							Error();
							return;
						}
					}

					istemplate = true;
					templates.add_goal();
					templates.goal[templates.last].set_name(buf);
				}
				else
				{
					fprintf(stderr,"%s: There can be only one percent in a template\n", buf);
					delete [] buf;
					Error();
					break;
				}
			}
			else
			{
				for(int i = 0; i <= goals.last; i++)
				{
					if(strcmp(buf, goals.goal[i].name) == 0)
					{
						fprintf(stderr,"Multiple definition of rule \"%s\"\n", buf);
						delete [] buf;
						Error();
						return;
					}
				}

				istemplate = false;
				goals.add_goal();
				goals.goal[goals.last].set_name(buf);
			}
			
			delete [] buf;
			left.add_value(lex_buf);
			
			getlex();
			Subgoals();
			break;

		case EQUALS:
			var_buf.set_name(buf);
			delete [] buf;

			getlex();
			Var_init();
			break;

		case VAR_CALL:
			found = vars.find(lex.string());
			
			if(found < 0)
			{
				fprintf(stderr,"No such variable at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
				Error();
			}
			else
			{
				lex_regime = found + 1;
				getlex();
				Name();
			}
			
			break;

		default:
			delete [] buf;
			fprintf(stderr,"Invalid lexeme type after name at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;
	}
}

void Subgoals()
{	
	switch(lex.type())
	{
		case WHITESPACE:
			right.add_value(lex);

			getlex();
			Subgoals();
			break;

		case NAME: case INCLUDE:
			right.add_value(lex);
			if(isphony)
			{
				phony.add_subgoal(lex.string());
				getlex();
				Subgoals();
				break;
			}
			if(istemplate)
			{
				if(strchr(lex.string(), '%') != strrchr(lex.string(), '%'))
				{					
					fprintf(stderr, "%s in %s: There can be only one percent in a template\n", lex.string(), templates.goal[templates.last].name);
					Error();
					break;
				}

				templates.goal[templates.last].add_subgoal(lex.string());
			}
			else
			{
				if(strchr(lex.string(), '%') != NULL)
				{
					fprintf(stderr, "%s in %s: No percents in non-template goals\n", lex.string(), goals.goal[goals.last].name);
					Error();
					break;
				}

				goals.goal[goals.last].add_subgoal(lex.string());
			}
			getlex();
			Subgoals();
			break;

		case NEWLINE:
			getlex();
			Command_tab();
			break;

		case VAR_CALL:
			found = vars.find(lex.string());
			
			if(found < 0)
			{
				fprintf(stderr,"No such variable at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
				Error();
			}
			else
			{
				lex_regime = found + 1;
				getlex();
				Subgoals();
			}
			
			break;
			
		default:
			fprintf(stderr,"Invalid lexeme type after semicolon at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;
	}
}

void Var_init()
{
	if(isalpha(var_buf.name[0]) || var_buf.name[0] == '_')
	{	
		for(int i = 1; var_buf.name[i] != '\0'; i++)
			if(!isalpha(var_buf.name[i]) && !isdigit(var_buf.name[i]) && var_buf.name[i] != '_')
			{
				
				fprintf(stderr, "Invalid variable init \"%s\"%d\n", var_buf.name, i);
				Error();
				return;
			}
	}
	else
	{
		fprintf(stderr, "Invalid variable init \"%s\"\n", var_buf.name);
		Error();
		return;
	}

	switch(lex.type())
	{
		case LEFT_PART: case RIGHT_PART: case VAR_CALL:
			fprintf(stderr, "Variable call in variable definition in (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;

		case NEWLINE:
			vars.add(var_buf);
			var_buf.del();
			getlex();
			break;

		default:
			var_buf.add_value(lex);
			getlex();
			Var_init();
			break;
	}
	return;	
}

void Include()
{
	static Lexeme lex_buf;
	
	static char * buf;
	if(lex.type() == INCLUDE)
	{
		buf = new char[strlen(lex.string()) + 1];
		strcpy(buf, lex.string());
		lex_buf = lex;
	}
	
	getlex();
	
	switch(lex.type())
	{
		case NAME:
			delete [] buf;
			Include_file();
			break;
		
		case WHITESPACE:
			Include();
			break;
		
		case SEMICOLON:
			goals.add_goal();
			goals.goal[goals.last].set_name(buf);
			delete [] buf;
			
			left.add_value(lex_buf);
			istemplate = false;	
			getlex();
			Subgoals();
			break;

		case EQUALS:
			var_buf.set_name(buf);
			delete [] buf;

			getlex();
			Var_init();
			break;

		case VAR_CALL:
			found = vars.find(lex.string());
			
			if(found < 0)
			{
				fprintf(stderr,"No such variable at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
				Error();
			}
			else
			{
				lex_regime = found + 1;
				getlex();
				Include();
			}
			
			break;

		default:
			delete [] buf;
			fprintf(stderr,"Invalid lexeme type after include at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;
	}
}

void Include_file()
{
	static char *buf;
	int res;

	if(lex.type() == NAME)
	{
		buf = new char[strlen(lex.string()) + 1];
		strcpy(buf, lex.string());
	}

	getlex();

	switch(lex.type())
	{
		case WHITESPACE:
			Include_file();
			break;

		case NEWLINE:
			
			res = files.add_file(buf, c);
			delete[] buf;
			if(res < 0)
			{
				Error();
				break;
			}
			c = gc();
			getlex();
			break;

		default:
			delete [] buf;
			fprintf(stderr,"Invalid lexeme type after include file at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;
	}
}

void Phony()
{
	static Lexeme lex_buf;
	
	static char * buf;
	if(lex.type() == PHONY)
	{
		buf = new char[strlen(lex.string()) + 1];
		strcpy(buf, lex.string());
		lex_buf = lex;
	}
	
	getlex();

	switch(lex.type())
	{
		case WHITESPACE:
			Phony();
			break;

		case SEMICOLON:
			phony.~Goal();

			phony.name = NULL;
			phony.subgoal = (char **) malloc(sizeof(char *));
			phony.subgoal[0] = NULL;
			phony.command = (char **) malloc(sizeof(char *));
			phony.command[0] = NULL;
			
			phony.set_name(buf);
			delete [] buf;
			
			left.add_value(lex_buf);
			
			istemplate = false;
			isphony = true;
			getlex();
			Subgoals();
			break;

		case VAR_CALL:
			found = vars.find(lex.string());
			
			if(found < 0)
			{
				fprintf(stderr,"No such variable at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
				Error();
			}
			else
			{
				lex_regime = found + 1;
				getlex();
				Phony();
			}
			
			break;

		default:
			delete [] buf;
			fprintf(stderr,"Invalid lexeme type after .PHONY file at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
			Error();
			break;
	}
}

void Command_tab()
{
	switch(lex.type())
	{
		case TAB:
			if(isphony)
			phony.add_command("");
			else if(istemplate)
				templates.goal[templates.last].add_command("");
			else
				goals.goal[goals.last].add_command("");
			
			getlex();
			Command();
			break;

		case END:
			break;

		case NEWLINE:
			getlex();
			//falls through

		default:
			left.del();
			right.del();
			isphony = false;
			istemplate = false;
			break;
	}
}

void Command()
{

	switch(lex.type())
	{
		case NEWLINE:
			getlex();
			isphony = false;
			istemplate = false;
			Command_tab();
			break;

		case VAR_CALL:
			found = vars.find(lex.string());
			
			if(found < 0)
			{
				fprintf(stderr,"No such variable at (%d, %d)\n", lex.lex_coordinates[0], lex.lex_coordinates[1]);
				Error();
			}
			else
			{
				lex_regime = found + 1;
				getlex();
				Command();
			}
			
			break;

		case END:
			break;

		case LEFT_PART:
			lex_regime = -2;
			getlex();
			Command();
			break;

		case RIGHT_PART:
			lex_regime = -1;
			getlex();
			Command();
			break;

		default:
			if(isphony)
				phony.add_to_command(lex.string());
			else if(istemplate)
				templates.goal[templates.last].add_to_command(lex.string());
			else
				goals.goal[goals.last].add_to_command(lex.string());
			
			getlex();
			Command();
			break;		
	}
}

void Error()
{
	lex.set_type(END);
	return;
}

//THIRD PART

int Build_Tree(Tree *node)
{
	int last_leaf = 0;
	static int res = 0;
	bool found = false;
	struct stat goal_f;
	struct stat subgoal_f;
	bool update = false;

	if(node->value == -1)
	{
		if(goals.last < 0)
			return 0;
		
		if(Argv[n] != NULL)
		{
			int i;
			for(i = 0; i <= goals.last; i++)
				if(strcmp(Argv[n], goals.goal[i].name) == 0)
				{
					found = true;
					break;
				}
			if(found)
				root.add_child(i);
			else
			{
				fprintf(stderr, "Could not find '%s'\n", Argv[n]);
				return -1;
			}
		}
		else
			root.add_child(0);

		if(Build_Tree(root.child[0]) > 0)
			root.del_child();

		if(res < 0)
			printf(" <- %s\n", goals.goal[0].name);
		return res;
	}
	
//	met_in_tree.value[node->value] = true;
	met_in_branch.value[node->value] = true;

//	printf("*Building Tree for '%s'*\n", goals.goal[node->value].name);

	bool goal_file = false;

	if(stat(goals.goal[node->value].name, &goal_f) == 0)
		if(S_ISREG(goal_f.st_mode))
			goal_file = true;

	update = !goal_file;

	for(int i = 0; goals.goal[node->value].subgoal[i] != NULL; i++)
	{
		found = false;
		int j;
		for(j = 0; j <= goals.last; j++)
		{
			if(strcmp(goals.goal[node->value].subgoal[i], goals.goal[j].name) == 0)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			//TODO TEMPLATE
			
			int k;
			char *nash_name = goals.goal[node->value].subgoal[i];
			char *temp_name;
			char *proc;
			char *rep_buf;
			int before;
			int	after;
			int rep_len;
			char *replace;

			for(k = 0; k <= templates.last; k++)
			{
				temp_name = templates.goal[k].name;
				proc = strchr(temp_name,'%');

				rep_len = strlen(nash_name) - strlen(temp_name) + 1;
				if(rep_len > 0)
				{
					before = proc - temp_name;
					after = &temp_name[strlen(temp_name) - 1] - proc;

					if(before == 0 || strncmp(nash_name, temp_name, before) == 0)
					{
						if(after == 0 || strcmp(&nash_name[before + rep_len], &temp_name[before + 1]) == 0)
						{
							found = true;
							replace = new char[rep_len + 1];
							strncpy(replace, &nash_name[before], rep_len);
							replace[rep_len] = '\0';
//							printf("'%s' -> '%s' (%% == '%s')\n", temp_name, nash_name, replace);
							break;
						}
					}
				}

			}
			if(found)
			{
				goals.add_goal();
				goals.goal[goals.last].set_name(nash_name);
				
				for(int kk = 0; (nash_name = templates.goal[k].subgoal[kk]) != NULL; kk++)
				{
					proc = strchr(nash_name, '%');

					if(proc != NULL)
					{
						rep_buf = new char[strlen(nash_name) + strlen(replace)];
						sprintf(rep_buf, "%s", nash_name);
						sprintf(&rep_buf[proc - nash_name], "%s", replace);
						sprintf(&rep_buf[proc - nash_name + strlen(replace)], "%s", &proc[1]);
//						printf("!%s!\n", rep_buf);
						goals.goal[goals.last].add_subgoal(rep_buf);
						delete [] rep_buf;
					}
					else
					{
						printf("!%s!\n", nash_name);
						goals.goal[goals.last].add_subgoal(nash_name);
					}
				}
				
				for(int kk = 0; (nash_name = templates.goal[k].command[kk]) != NULL; kk++)
				{
					char * rep_buf2 = new char[strlen(nash_name) + 1];
					strcpy(rep_buf2, nash_name);
					
					for(;(proc = strchr(rep_buf2, '%')) != NULL;)
					{
						
						rep_buf = new char[strlen(rep_buf2) + strlen(replace)];
						sprintf(rep_buf, "%s", rep_buf2);
						sprintf(&rep_buf[proc - rep_buf2], "%s", replace);
						sprintf(&rep_buf[proc - rep_buf2 + strlen(replace)], "%s", &proc[1]);
						delete [] rep_buf2;
						rep_buf2 = rep_buf;
					}
//					printf("\t@%s@\n", rep_buf2);
					goals.goal[goals.last].add_command(rep_buf2);
					delete [] rep_buf2;
					
				}

				delete [] replace;
				
				j = goals.last;

				met_in_branch.add(true);
				met_in_tree.add(false);

			}
			//TODO TEMPLATE
			else
			{
				struct stat subsub;
				if(goal_file)
					if(stat(goals.goal[node->value].name, &subsub) >=0)
						if(S_ISREG(subsub.st_mode))
							if(subsub.st_mtime <= goal_f.st_mtime)
							{
								return 1;
							}
				if(res == -1)
				fprintf(stderr, "Rule not found: %s", goals.goal[node->value].subgoal[i]);
				return res;
			}
		}
		else if(met_in_branch.value[j])
		{
			res = -2;
			fprintf(stderr, "Recursion: %s", goals.goal[node->value].subgoal[i]);
			return res;
		}

//		if(!met_in_tree.value[j])
//		{

			if(goal_file)
				if(stat(goals.goal[j].name, &subgoal_f) == 0)
					if(S_ISREG(subgoal_f.st_mode))
						if(subgoal_f.st_mtime <= goal_f.st_mtime)
						{
						}
						else update = true;
					else update = true;
				else update = true;
			else update = true;
			
			node->add_child(j);
			if(Build_Tree(node->child[last_leaf++]) > 0)
			{
				node->del_child();
				last_leaf--;
			}

			if(res < 0)
			{
				printf(" <- %s", goals.goal[node->value].subgoal[i]);
				return res;
			}
//		}
	}

	met_in_branch.value[node->value] = false;

	for(int i = 0; phony.subgoal[i] != NULL; i++)
		if(strcmp(goals.goal[node->value].name, phony.subgoal[i]) == 0)
			update = true;

	if(node->child[0] == NULL && update == false)
	{
//		printf("|No need to execute %s|\n", goals.goal[node->value].name);
		return 1;
	}
	return res;
}

void Execute_Tree(Tree *node)
{
	if(node->value >= 0)
	{
		if(met_in_tree.value[node->value] == true)
			return;
	}
	else if(node->child[0] == NULL)
	{
		printf("Nothing to build\n");
		return;
	}
	
	for(int i = 0; node->child[i] != NULL; i++)
		Execute_Tree(node->child[i]);
	
	if(node->value >= 0)
	{
		for(int i = 0; goals.goal[node->value].command[i] != NULL; i++)
		{
			printf("%s\n",goals.goal[node->value].command[i]);
			system(goals.goal[node->value].command[i]);
		}
		met_in_tree.value[node->value] = true;
	}

}
