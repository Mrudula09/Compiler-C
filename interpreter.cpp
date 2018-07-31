#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <malloc.h>
#include<string.h>
#include<stdlib.h>

#define SYMBOLCOUNT 17
#define REGISTERCOUNT 8

int table_size=0, memory_index=0, instruction_num=0, block_num=0;
char symbols[18][7] = { "DATA","CONST","MOV", "ADD", "SUB", "MUL", "IF", "THEN", "ELSE", "ENDIF", "JUMP", "READ", "PRINT", "GT", "LT", "EQ", "GTEQ", "LTEQ" };
char registers[8][3] = { "AX", "BX", "CX", "DX", "EX", "FX", "GX", "HX" };

struct symboltable
{
	char* name;
	int address;
	int size;
};

struct block
{
	char* name;
	int address;
};

struct stack
{
	int top = 0;
	int* array;
};

struct intermediate
{
	int opcode;
	int* parameters;
	int no_of_parameters;
};

//to return opcode of a token if it is a symol , -1 is not a symbol
int opcode(char* token)
{
	int i;
	for (i = 0; i < SYMBOLCOUNT; i++)
	{
		if (!strcmp(symbols[i], token))
		{
			return i;
		}
	}
	return -1;
}

//returns index of register 
int isRegister(char* token)
{
	int i = 0;
	for (i = 0; i < REGISTERCOUNT; i++)
	{
		if (!strcmp(registers[i], token))
			return i;
	}
	return -1;
}

void data(char* buf, int index,struct symboltable* symboltable)
{
	int i = 0,k=0,size=1;
	char* name = (char*)malloc(sizeof(char) * 50);
	while (buf[index] != '\n'&&buf[index] != '\0')
	{
		if (buf[index] == '[')
		{
			size = buf[index + 1]-'0';
			break;
		}
		name[k] = buf[index];
		index++;
		k++;
	}
	name[k] = '\0';
	symboltable[table_size].name = (char*)malloc(sizeof(char) * 50);
	strcpy(symboltable[table_size].name, name);
	symboltable[table_size].address = memory_index;
	symboltable[table_size].size = size;
	table_size++;
	memory_index = memory_index + size;
}

void cons(char* buf, int index, struct symboltable* symboltable,int* memory)
{
	int i = 0, k = 0, size = 1,value=0;
	char* name = (char*)malloc(sizeof(char) * 50);
	while (buf[index] != '\n'&&buf[index] != '\0'&&buf[index]!=' ')
	{
		name[k] = buf[index];
		index++;
		k++;
	}
	name[k] = '\0';
	index++;
	if (buf[index] == '=')
	{
		index += 2;
		while (buf[index] != '\n'&&buf[index] != '\0'&&buf[index] != ' ')
		{
			value = value * 10 + (buf[index] - '0');
			index++;
		}
	}
	symboltable[table_size].name = (char*)malloc(sizeof(char) * 50);
	strcpy(symboltable[table_size].name, name);
	symboltable[table_size].address = memory_index;
	symboltable[table_size].size = 0;
	memory[memory_index] = value;
	memory_index++;
	table_size++;
}

char* get_token(char* buf, int *index)
{
	int i = 0, k = 0, size = 1, value = 0;
	char* name = (char*)malloc(sizeof(char) * 50);
	while (buf[*index] != '\n'&&buf[*index] != '\0'&&buf[*index] != ','&&buf[*index] != ' ')
	{
		if (buf[*index] == '\t')
		{
			(*index)++;
			continue;
		}
		name[k] = buf[*index];
		(*index)++;
		k++;
	}
	name[k] = '\0';
	return name;
}

int search_symboltable(char* name,struct symboltable* symboltable)
{
	int i;
	for (i = 0; i < table_size; i++)
	{
		if (!strcmp(name, symboltable[i].name))
			return i;
	}
	return -1;
}
void move(char* buf, int index, struct intermediate* opcodetable, struct symboltable* symboltable)
{
	char* name;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters = (int*)malloc(sizeof(int) * 4);
	int i = isRegister(name);
	if (i>0)
	{
		opcodetable[instruction_num].parameters[0] = i;
		//register to memory 
		opcodetable[instruction_num].parameters[2] = 1;
		index++;
		name = get_token(buf, &index);
		i = search_symboltable(name, symboltable);
		opcodetable[instruction_num].parameters[1] = i;
	}
	else
	{
		i = search_symboltable(name,symboltable);
		opcodetable[instruction_num].parameters[0] = i;
		opcodetable[instruction_num].parameters[2] = 0;
		index++;
		name = get_token(buf, &index);
		i = isRegister(name);
		opcodetable[instruction_num].parameters[1] = i;
	}
	opcodetable[instruction_num].no_of_parameters = 3;
	instruction_num++;
}

void arthematic_operations(char* buf, int index, struct symboltable* symboltable, struct intermediate* opcodetable)
{
	char* name;
	opcodetable[instruction_num].parameters = (int*)malloc(sizeof(int) * 4);
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters[0] = isRegister(name);
	index++;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters[1] = isRegister(name);
	index++;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters[2] = isRegister(name);
	opcodetable[instruction_num].no_of_parameters = 3;
	instruction_num++;
}

void  read_or_write(char* buf, int index, struct symboltable* symboltable,struct intermediate* opcodetable)
{
	char *name;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters = (int*)malloc(sizeof(int) * 4);
	int temp = isRegister(name);
	if (temp >= 0)
	{
		opcodetable[instruction_num].parameters[0] = temp;
		opcodetable[instruction_num].parameters[1] = 1;
	}
	else
	{
		opcodetable[instruction_num].parameters[0] = search_symboltable(name, symboltable);
		opcodetable[instruction_num].parameters[1] = 0;
	}
	opcodetable[instruction_num].no_of_parameters = 2;
	instruction_num++;
}

void save_block_pointer(char* name,struct block* block)
{
	block[block_num].name = (char*)malloc(sizeof(char) * 10);
	strcpy(block[block_num].name, name);
	block[block_num].address = instruction_num + 1;
	block_num++;
}

int search_block(char* name,struct block* block)
{
	int i;
	for (i = 0; i < block_num; i++)
	{
		if (!strcmp(block[i].name, name))
		{
			return block[i].address;
		}
	}
	return -1;
}

void jump(char* buf, int index, struct block* block,struct intermediate* opcode)
{
	char* name;
	name = get_token(buf, &index);
	opcode[instruction_num].parameters = (int*)malloc(sizeof(int) * 4);
	opcode[instruction_num].parameters[0] = search_block(name, block);
	opcode[instruction_num].no_of_parameters = 1;
	instruction_num++;
}

void if_block(char* buf,int index,struct symboltable* symboltable,struct intermediate* opcodetable,struct stack* stack)
{
	char *name;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters = (int*)malloc(sizeof(int) * 4);
	opcodetable[instruction_num].parameters[0] = isRegister(name);
	index++;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters[2] = opcode(name);
	index++;
	name = get_token(buf, &index);
	opcodetable[instruction_num].parameters[1] = isRegister(name);
	opcodetable[instruction_num].parameters[3] = -1;
	opcodetable[instruction_num].no_of_parameters = 4;
	stack->top=0;
	stack->array = (int*)malloc(sizeof(int) * 10);
	stack->array[stack->top] = instruction_num;
	instruction_num++;
}

void else_block(char* buf, int index, struct symboltable* symboltable, struct intermediate* opcode, struct stack* stack)
{
	opcode[instruction_num].parameters = (int*)malloc(sizeof(int) * 4);
	opcode[stack->array[stack->top]].parameters[3] = instruction_num + 1;
	opcode[instruction_num].no_of_parameters = 1;
	stack->array[stack->top] = instruction_num;
	instruction_num++;
}

void endif_block(char* buf, int index, struct symboltable* symboltable, struct intermediate* opcode, struct stack* stack)
{
	opcode[stack->array[stack->top]].parameters[0] = instruction_num;
	stack->top = stack->top - 1;
}

void execute_move(int instruction_num,struct symboltable* symboltable, struct intermediate* opcode, struct block* block,int* memory,int* register_data)
{
	int memory_index,register_index;
	if (opcode[instruction_num].parameters[2] == 0)
	{
		memory_index = symboltable[opcode[instruction_num].parameters[0]].address;
		register_index = symboltable[opcode[instruction_num].parameters[1]].address;
		memory[memory_index] = register_data[register_index];
	}
	else
	{
		memory_index = symboltable[opcode[instruction_num].parameters[1]].address;
		register_index = symboltable[opcode[instruction_num].parameters[0]].address;
		register_data[register_index] = memory[memory_index];
	}
}

void execute_add(int i, struct symboltable* symboltable, struct intermediate* opcode, struct block* block,int* register_data)
{
	register_data[opcode[i].parameters[0]] = register_data[opcode[i].parameters[1]] + register_data[opcode[i].parameters[2]];
}

void execute_sub(int i, struct symboltable* symboltable, struct intermediate* opcode, struct block* block, int* register_data)
{
	register_data[opcode[i].parameters[2]] = register_data[opcode[i].parameters[0]] - register_data[opcode[i].parameters[1]];
}

void execute_mul(int i, struct symboltable* symboltable, struct intermediate* opcode, struct block* block, int* register_data)
{
	register_data[opcode[i].parameters[2]] = register_data[opcode[i].parameters[0]] * register_data[opcode[i].parameters[1]];
}

void execute_read(int i,struct symboltable* symboltable,struct intermediate* opcode,int* memory,int* register_data)
{
	if (opcode[i].parameters[1] == 1)
	{
		scanf("%d", &register_data[opcode[i].parameters[0]]);
	}
	else
	{
		scanf("%d", &memory[opcode[i].parameters[0]]);
	}
}

void execute_print(int i, struct symboltable* symboltable, struct intermediate* opcode, int* memory, int* register_data)
{
	if (opcode[i].parameters[1] == 1)
	{
		printf("\n%d\n",register_data[opcode[i].parameters[0]]);
	}
	else
	{
		printf("\n%d\n",memory[opcode[i].parameters[0]]);
	}
}

void execute_if(int i, struct symboltable* symboltable, struct intermediate* opcode, int* memory, int* register_data,int *instruction)
{
	int a, b;
	a = opcode[i].parameters[0];
	b = opcode[i].parameters[1];
	if (opcode[i].parameters[2] == 13)
	{
		if (register_data[a] > register_data[b])
			return;
		else
			*instruction = opcode[i].parameters[3]-1;
	}
	else if(opcode[i].parameters[2] == 14)
	{
		if (register_data[a] < register_data[b])
			*instruction = i + 1;
		else
			*instruction = opcode[i].parameters[3];
	}
	else if (opcode[i].parameters[2] == 15)
	{
		if (register_data[a] == register_data[b])
			*instruction = i + 1;
		else
			*instruction = opcode[i].parameters[3];
	}
	else if (opcode[i].parameters[2] == 16)
	{
		if (register_data[a] >= register_data[b])
			*instruction = i + 1;
		else
			*instruction = opcode[i].parameters[3];
	}
	else if (opcode[i].parameters[2] == 17)
	{
		if (register_data[a] <= register_data[b])
			*instruction = i + 1;
		else
			*instruction = opcode[i].parameters[3];
	}
}


void execute_else(int i, struct intermediate* opcodetable, int *instruction)
{
	*instruction = opcodetable[i].parameters[0];
}

void execute_jump(int i,struct intermediate* opcode, int *instruction)
{
	*instruction = opcode[i].parameters[0];
}

void execute(struct symboltable* symboltable, struct intermediate* opcodetable, struct block* block,int* memory,int* register_data)
{
	int i = 0;
	for (i = 0; i <= instruction_num; i++)
	{
		switch (opcodetable[i].opcode)
		{
		case 2:
			execute_move(i, symboltable, opcodetable, block, memory, register_data);
			break;
		case 3:
			execute_add(i, symboltable, opcodetable, block, register_data);
			break;
		case 4:
			execute_sub(i, symboltable, opcodetable, block, register_data);
			break;
		case 5:
			execute_mul(i, symboltable, opcodetable, block, register_data);
			break;
		case 6:
			execute_if(i, symboltable, opcodetable, memory, register_data, &i);
			break;
		case 8:
			execute_else(i, opcodetable, &i);
			break;
		case 10:
			execute_jump(i, opcodetable, &i);
			break;
		case 11:
			execute_read(i, symboltable, opcodetable, memory, register_data);
			break;
		case 12:
			execute_print(i, symboltable, opcodetable, memory, register_data);
			break;
		}
	}
}

int write_opcode_into_file(struct intermediate* opcodetable)
{
	FILE *outfile;
	outfile = fopen("opcode.dat", "w");
	if (outfile == NULL)
	{
		fprintf(stderr, "\nError opend file\n");
		exit(1);
	}
	fwrite(opcodetable, sizeof(opcodetable), 1, outfile);
	if (fwrite != 0)
		printf("contents to file written successfully !\n");
	else
		printf("error writing file !\n");
	return 0;
}

int write_table_into_file(struct symboltable* table)
{
	FILE *outfile;
	outfile = fopen("symboltable.dat", "w");
	if (outfile == NULL)
	{
		fprintf(stderr, "\nError opend file\n");
		exit(1);
	}
	fwrite(table, sizeof(table), 1, outfile);
	if (fwrite != 0)
		printf("contents to file written successfully !\n");
	else
		printf("error writing file !\n");
	return 0;
}

int write_block_into_file(struct block* block)
{
	FILE *outfile;
	outfile = fopen("bloack.dat", "w");
	if (outfile == NULL)
	{
		fprintf(stderr, "\nError opend file\n");
		exit(1);
	}
	fwrite(block, sizeof(block), 1, outfile);
	if (fwrite != 0)
		printf("contents to file written successfully !\n");
	else
		printf("error writing file !\n");
	return 0;
}

int load_meta_data()
{
	FILE *fp;
	fp = fopen("meta.txt", "w");
	fprintf(fp, "%d", table_size);
	fprintf(fp, "%d", instruction_num);
	fprintf(fp, "%d", block_num);
	fprintf(fp, "%d", SYMBOLCOUNT);
	fprintf(fp, "%d", REGISTERCOUNT);
	return 0;
}

int main()
{
	int* memory = (int*)malloc(sizeof(int) * 100);
	FILE *fp = fopen("compiler_test.txt","r");
	char* buf = (char*)malloc(sizeof(char)*100);
	char* token = (char*)malloc(sizeof(char)*100);
	int* register_data = (int*)malloc(sizeof(int) * 10);
	struct symboltable* symboltable = (struct symboltable*)malloc(sizeof(struct symboltable)*20);
	struct intermediate* opcodetable = (struct intermediate*)malloc(sizeof(struct intermediate)*40);
	struct block* block = (struct block*)malloc(sizeof(struct block) * 20);
	struct stack* stack = (struct stack*)malloc(sizeof(struct stack) * 20);
	int i,k,op,j;
	while (fgets(buf, 100, fp))
	{
		k = 0;
		for (i = 0; buf[i] != '\n'&&buf[i] != ' '&&buf[i]!=':';i++)
		{
			if (buf[i] == '\t')
				continue;
			token[k] = buf[i];
			k++;
		}
		token[k] = '\0';
		op = opcode(token);
		switch (op)
		{
		case 0:
			data(buf, i+1,symboltable);
			break;
		case 1:
			cons(buf, i+1,symboltable,memory);
			break;
		case 2:
			opcodetable[instruction_num].opcode = 2;
			move(buf, i + 1,opcodetable,symboltable);
			break;
		case 3:
			opcodetable[instruction_num].opcode = 3;
			arthematic_operations(buf, i + 1, symboltable, opcodetable);
			break;
		case 4:
			opcodetable[instruction_num].opcode = 4;
			arthematic_operations(buf, i + 1, symboltable, opcodetable);
			break;
		case 5:
			opcodetable[instruction_num].opcode = 5;
			arthematic_operations(buf, i + 1, symboltable, opcodetable);
			break;
		case 10:
			opcodetable[instruction_num].opcode = 10;
			jump(buf, i + 1, block, opcodetable);
			break;
		case 11:
			opcodetable[instruction_num].opcode = 11;
			read_or_write(buf, i + 1, symboltable, opcodetable);
			break;
		case 12:
			opcodetable[instruction_num].opcode = 12;
			read_or_write(buf, i + 1, symboltable, opcodetable);
			break;
		case 13:
			opcodetable[instruction_num].opcode = 13;
			read_or_write(buf, i + 1, symboltable, opcodetable);
			break;
		case -1:
			save_block_pointer(token,block);
			break;
		case 6:
			opcodetable[instruction_num].opcode = 6;
			if_block(buf,i+1,symboltable,opcodetable,stack);
			break;
		case 8:
			opcodetable[instruction_num].opcode = 8;
			else_block(buf, i + 1, symboltable, opcodetable, stack);
			break;
		case 9:
			opcodetable[instruction_num].opcode = 9;
			endif_block(buf, i + 1, symboltable, opcodetable, stack);
			break;
		default:
			printf("syntax error");
			break;
		}
	}
	printf("Symbol table\n");
	for (i = 0; i < table_size; i++)
	{
		printf("%s\t%d\t%d\n", symboltable[i].name,symboltable[i].address,symboltable[i].size);
	}
	printf("Intermediate code\n");
	for (i = 0; i < instruction_num; i++)
	{
		printf("%d\t%d\t", i, opcodetable[i].opcode);
		for (j = 0; j < opcodetable[i].no_of_parameters; j++)
		{
			printf("%d\t", opcodetable[i].parameters[j]);
		}
		printf("\n");
	}
	printf("\n\n");
	execute(symboltable,opcodetable,block,memory,register_data);
	fclose(fp);
	write_table_into_file(symboltable);
	write_opcode_into_file(opcodetable);
	write_block_into_file(block);
	load_meta_data();
	return 0;
}