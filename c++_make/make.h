#define END			(-1)
#define	NAME		0
#define	TAB			1
#define	WHITESPACE	2
#define	SEMICOLON	3
#define	RIGHT_PART	4
#define	LEFT_PART	5
#define	INCLUDE		6
#define	PHONY		7
#define	EQUALS		8
#define	VAR_CALL	9
#define	NEWLINE		10

#define	START		11
#define READ		12
#define	DOLLAR 		13
#define	COMMENT 	14
#define	VAR		 	15

class Lexeme
{
	public:
		int lex_type;
		char *lex_string;
		int lex_coordinates[4];
//	public:
		Lexeme();
		~Lexeme();

		void set_type(int);
		void set_string(const char *);
		void set_coord(int, int, int, int);

		Lexeme & operator =(Lexeme &);
		
		int type(void);
		char* string(void);
		int* coord(void);

		void print(void);
};

class Lex_str
{
	public:
	
		char *str;

		Lex_str();
		~Lex_str();

		void add(int);
		void del();
};

class Goal
{
	public:
		char *name;
		char **subgoal;
		char **command;

		Goal();
		~Goal();

		void set_name(const char *);
		void add_subgoal(const char *);
		void add_command(const char *);
		void add_to_command(const char *);
};

class Goals
{
	public:
		Goal *goal;
		int last;
		
		Goals();
		~Goals();

		void add_goal();
};

class Var
{
	public:
		char *name;
		Lexeme *value;
		
		void set_name(const char *);
		void add_value(Lexeme &);
		void del();
		
		Var & operator =(Var &);

		Var();
		~Var();
};

class Vars
{
	public:
		Var *var;

		int find(const char *);
		void add(Var &);

		Vars();
		~Vars();
};

class Files
{
	public:

		char **path;
		FILE **file;
		int *c_buf;

		int last;

		Files();
		~Files();
		
		int add_file(const char *, int);
		int close_one(void);
};

class Tree
{
	public:
		int value;
		Tree *parent;
		Tree **child;

		Tree();
		~Tree();

		void construct();
		void add_child(int);
		void del_child();
		void print(int);

};

class Array
{
	public:
		bool *value;
		int size;
	
		Array(int);
		~Array();
		
		void add(bool);
		void reset();
};
