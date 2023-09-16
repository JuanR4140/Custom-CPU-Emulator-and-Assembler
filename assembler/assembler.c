#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../emu/cpu.h"

int line_number = 0;

typedef struct Label{
  char name[256];
  int address;
} Label;

int find_label(char* label_name, Label* symbol_table, int table_size){
  for(int i = 0; i < table_size; i++){
    if(strcmp(label_name, symbol_table[i].name) == 0){
      return symbol_table[i].address;
    }
  }
  return -1;
}

void throw_error(char* expected, char* got, int line_number){
  printf("Error at line %i.\nExpected %s, but got '%s'.\n", line_number, expected, got);
  exit(1);
}

void sanitize_line(char* line){
  // Find first valid character
  char* start = line;
  while (*start && (*start == ' ' || *start == '\t')) {
    start++;
  }

  // Find first ';' (comment)
  char* comment_start = strchr(start, ';');

  if (comment_start != NULL) {
    // If comment, remove it with null terminator
    *comment_start = '\0';
  }

  // Remove trailing whitespace
  int len = strlen(start);
  while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t')) {
    start[--len] = '\0';
  }
  // Remove newline if it exists
  if(start[strlen(start)-1] == '\n') start[strlen(start)-1] = '\0';
  // Copy modified string to original
  strcpy(line, start);
}

long convert_to_decimal(char* input){
  char* endptr;
  long result;

  if (input[0] == '$') {
    // Skip the '$' character if it's present
    input++;
  }

  // Try to convert the input string to a long using strtol
  result = strtol(input, &endptr, 0);

  // Check for conversion errors
  if(endptr == input) {
    // printf("Error: Invalid number format\n");
    throw_error("valid number", input, line_number);
    return 0; // or some other error handling mechanism
  }

  if(result > 0xFFFF) throw_error("16-bit number", input, line_number);

  return result;
}

unsigned char register_to_machine_code(char* reg){
  if(strcmp(reg, "ax") == 0){
    return 0x0;
  }else if(strcmp(reg, "bx") == 0){
    return 0x1;
  }else if(strcmp(reg, "cx") == 0){
    return 0x2;
  }else if(strcmp(reg, "dx") == 0){
    return 0x3;
  }else{
    // printf("ERROR REGISTER EXPECTED.\n");
    throw_error("valid register", reg, line_number);
    exit(0);
  }
}

int find_character(char* line, int size, char character){
  for(int i = 0; i < size; i++){
    if(line[i] == character) return i;
  }
  return -1;
}

int check_label_redefinition(Label* symbol_table, int* table_size, char* line){
  for(int i = 0; i < *table_size; i++){
    if(strcmp(symbol_table[i].name, line) == 0) return 1;
  }
  return 0;
}

int resolve_label(char* line, Label* symbol_table, int* table_size){
  for(int i = 0; i < *table_size; i++){
    if(strcmp(symbol_table[i].name, line) == 0) return symbol_table[i].address;
  }
  return 0;
}

void store_label(char* line, Label* symbol_table, int* table_size, int* current_address){
  char line_copy[256];
  strcpy(line_copy, line);
  int pos = 0;
  sanitize_line(line_copy);

  if((pos = find_character(line_copy, strlen(line_copy), ':')) != -1){
    line_copy[pos] = '\0';
    if(check_label_redefinition(symbol_table, table_size, line_copy)){
      char err[512];
      sprintf(err, "redefinition of %s", line_copy);
      throw_error("unique label", err, line_number);
    }
    strcpy(symbol_table[*table_size].name, line_copy);
    symbol_table[*table_size].address = *current_address;
    (*table_size)++;
  }else{
    char* ins = strtok(line_copy, " ");
    char* reg = strtok(NULL, ", ");
    char* val = strtok(NULL, " ");
    if(ins == NULL && reg == NULL && val == NULL) return;
    *current_address += 4;
  }
  return;
}

int assemble_instruction(char* line, int* line_number, unsigned char* machine_code, Label* symbol_table, int* table_size, int* current_address){
  char line_copy[256];
  strcpy(line_copy, line);
  int pos = 0;
  sanitize_line(line_copy);
  if(*line_number == 1 && strcmp(line_copy, "_main:")){
    // printf("_main must be first line of code. Got '%s'\n\n(definitely not because im lazy.. haha..)\n", line_copy);
    throw_error("_main: to be at first line of code", line_copy, *line_number);
    exit(1);
  } 

  if((pos = find_character(line_copy, strlen(line_copy), ':')) != -1){
    // Line is label, add to symbol table.
    // printf("%X: %s\n", *current_address, line_copy);
    /*line_copy[pos] = '\0';
    strcpy(symbol_table[*table_size].name, line_copy);
    symbol_table[*table_size].address = *current_address;
    (*table_size)++;*/
  }else if((pos = find_character(line_copy, strlen(line_copy), ',')) != 1){
    // Line is (most likely) instruction, form machine code.
    // printf("%X: %s", *current_address, line_copy);
    char* ins = strtok(line_copy, " ");
    char* reg = strtok(NULL, ", ");
    char* val = strtok(NULL, " ");

    if(ins == NULL && reg == NULL && val == NULL) return 0;
    if(ins == NULL || reg == NULL && strcmp(ins, "ret") && strcmp(ins, "nop") && strcmp(ins, "mul") && strcmp(ins, "div") && strcmp(ins, "hlt") && strcmp(ins, "nop")){
      char err[32];
      snprintf(err, 32, "%s %s, %s", (ins ? ins : ""), (reg ? reg : ""), (val ? val : ""));
      throw_error("valid instruction", err, *line_number);
    }
        
    if(strcmp(ins, "mov") == 0){
      
      if(find_character(val, strlen(val), '$') != -1){
        // Moving addr to reg.
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x02;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }else if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0){
        // Moving reg to reg.
        machine_code[0] = 0x01;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        // Assume moving val to reg.
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x00;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }

      // printf("%s %s, %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, val, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);

    }else if(strcmp(ins, "str") == 0){

      if(find_character(val, strlen(val), '$') != -1){
        // Storing reg to addr
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x13;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }else{
        // Assume storing reg to reg at addr.
        machine_code[0] = 0x10;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }

      // printf("%s %s, %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, val, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);

    }else if(strcmp(ins, "sth") == 0){

      if(find_character(val, strlen(val), '$') != -1){
        // Storing high reg to addr
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x14;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }else{
        // Assume storing high reg to reg at addr.
        machine_code[0] = 0x11;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }

      // printf("%s %s, %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, val, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);

    }else if(strcmp(ins, "stl") == 0){

      if(find_character(val, strlen(val), '$') != -1){
        // Storing low reg to addr
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x15;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }else{
        // Assume storing low reg to reg at addr
        machine_code[0] = 0x12;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }

      // printf("%s %s, %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, val, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);

    }else if(strcmp(ins, "cmp") == 0){

      if(find_character(val, strlen(val), '$') != -1){
        // Comparing register to address
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x22;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }else if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0){
        // Comparing register to register
        machine_code[0] = 0x21;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        // Assume comparing register to val
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x20;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }

      // printf("%s %s, %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, val, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);

    }else if(strcmp(ins, "jmp") == 0){

      int label = resolve_label(reg, symbol_table, table_size);
      if(!label){
        throw_error("valid label", reg, *line_number);
      }
      // Store in mem.
      machine_code[0] = 0x30;
      machine_code[1] = 0x0;
      machine_code[2] = label & 0xFF;
      machine_code[3] = (label >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
    
    }else if(strcmp(ins, "je") == 0){

      int label = resolve_label(reg, symbol_table, table_size);
      if(!label){
        throw_error("valid label", reg, *line_number);
      }
      machine_code[0] = 0x31;
      machine_code[1] = 0x0;
      machine_code[2] = label & 0xFF;
      machine_code[3] = (label >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "jne") == 0){

      int label = resolve_label(reg, symbol_table, table_size);
      if(!label){
        throw_error("valid label", reg, *line_number);
      }
      machine_code[0] = 0x32;
      machine_code[1] = 0x0;
      machine_code[2] = label & 0xFF;
      machine_code[3] = (label >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "jl") == 0){

      int label = resolve_label(reg, symbol_table, table_size);
      if(!label){
        throw_error("valid label", reg, *line_number);
      }
      machine_code[0] = 0x33;
      machine_code[1] = 0x0;
      machine_code[2] = label & 0xFF;
      machine_code[3] = (label >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "jg") == 0){

      int label = resolve_label(reg, symbol_table, table_size);
      if(!label){
        throw_error("valid label", reg, *line_number);
      }
      machine_code[0] = 0x34;
      machine_code[1] = 0x0;
      machine_code[2] = label & 0xFF;
      machine_code[3] = (label >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "call") == 0){

      int label = resolve_label(reg, symbol_table, table_size);
      if(!label){
        throw_error("valid label", reg, *line_number);
      }
      machine_code[0] = 0x35;
      machine_code[1] = 0x0;
      machine_code[2] = label & 0xFF;
      machine_code[3] = (label >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "ret") == 0){

      int res;
      if(reg == NULL){
        res = 0x0;
      }else{
        res = convert_to_decimal(reg);
      }
      machine_code[0] = 0x36;
      machine_code[1] = 0x0;
      machine_code[2] = res & 0xFF;
      machine_code[3] = (res >> 8) & 0xFF;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "add") == 0){

      if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0){
        machine_code[0] = 0x41;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x40;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "sub") == 0){

      if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0){
        machine_code[0] = 0x51;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x50;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "mul") == 0){

      machine_code[0] = 0x60;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = 0x0;
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "div") == 0){

      machine_code[0] = 0x70;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = 0x0;
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "push") == 0){
      
      if(strcmp(reg, "ax") == 0 || strcmp(reg, "bx") == 0 || strcmp(reg, "cx") == 0 || strcmp(reg, "dx") == 0){
        machine_code[0] = 0x81;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = 0x0;
        machine_code[3] = 0x0;
      }else{
        int decimal = convert_to_decimal(reg);
        machine_code[0] = 0x80;
        machine_code[1] = 0x0;
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "pop") == 0){
      
      machine_code[0] = 0x90;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = 0x0;
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "and") == 0){

      machine_code[0] = 0xA0;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = register_to_machine_code(val);
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "not") == 0){
      
      machine_code[0] = 0xA1;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = 0x0;
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "or") == 0){

      machine_code[0] = 0xA2;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = register_to_machine_code(val);
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "xor") == 0){
      
      machine_code[0] = 0xA3;
      machine_code[1] = register_to_machine_code(reg);
      machine_code[2] = register_to_machine_code(val);
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "shr") == 0){

      if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0){
        machine_code[0] = 0xB1;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0xB0;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "shl") == 0){

      if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0){
        machine_code[0] = 0xB3;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0xB2;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "hlt") == 0){

      machine_code[0] = 0xFE;
      machine_code[1] = 0x0;
      machine_code[2] = 0x0;
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else if(strcmp(ins, "nop") == 0){

      machine_code[0] = 0xFF;
      machine_code[1] = 0x0;
      machine_code[2] = 0x0;
      machine_code[3] = 0x0;
      // printf("%s %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, (reg ? reg : ""), machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      
    }else{
      throw_error("valid mnemonic", ins, *line_number);
    }

    /*if(strcmp(ins, "mov") == 0){
      machine_code[0] = INS_MOV_VAL;
      machine_code[1] = register_to_machine_code(reg);
      int decimal = convert_to_decimal(val);
      machine_code[2] = decimal & 0xFF; // lsb
      machine_code[3] = (decimal >> 8) & 0xFF; // msb
      // printf("%s %s, %s --> 0x%X 0x%X 0x%X 0x%X\n", ins, reg, val, machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      c++;
      if(c == 3) exit(0);
    }*/
    
    // // printf("%s %s %s\n", ins, reg, val);
    *current_address += 4;
    return 4;
  }
  
  return 0;
}

int main(int argc, char* argv[]){
  if(argc != 3 && argc != 4){
    printf("Usage: %s input_file output_file\n", argv[0]);
    return 1;
  }

  char* input_file_name = argv[1];
  char* output_file_name = argv[2];

  char line[256];

  FILE* input_file = fopen(input_file_name, "r");
  if(input_file == NULL) throw_error("valid input file", input_file_name, line_number);
  FILE* output_file = fopen(output_file_name, "wb");

  unsigned char machine_code[4];

  Label symbol_table[256];
  int table_size = 0;

  // int current_address = 0xE000;
  int current_address;
  int origin_address;
  if(argc == 4){
    origin_address = convert_to_decimal(argv[3]);
    current_address = convert_to_decimal(argv[3]);
    printf("Code offset set at 0x%X.\n", current_address);
    printf("***WARNING***\nSetting a different code offset other than the\n");
    printf("recommended one (0xE000) can cause your program to not execute at all!!1\n");
    printf("You are running the risk of overwriting core CPU components!!\n");
    printf("Do not proceed if you are simply making a user program.\n\n");
  }else{
    origin_address = 0xE000;
    current_address = 0xE000;
  }
  
  // int line_number = 0;

  while(fgets(line, sizeof(line), input_file) != NULL){
    line_number++;
    // printf("OLD %s\n", line);
    store_label(line, symbol_table, &table_size, &current_address);
    // printf("NEW %s\n", line);
  }
  // printf("\n\n\n");
  for(int i = 0; i < table_size; i++){
    // printf("%s -> 0x%X\n", symbol_table[i].name, symbol_table[i].address);
  }

  // exit(0);
  fseek(input_file, 0, SEEK_SET);
  
  // current_address = 0xE000;
  current_address = origin_address;
  line_number = 0;
  int bytes_written = 0;
  while(fgets(line, sizeof(line), input_file) != NULL){
    line_number++;
    // // printf("%s", line);
    int code_length = assemble_instruction(line, &line_number, machine_code, symbol_table, &table_size, &current_address);
    if(code_length == 4){
      // printf("Machine code: %X %X %X %X\n", machine_code[0], machine_code[1], machine_code[2], machine_code[3]);
      fwrite(machine_code, 1, 4, output_file);
      bytes_written += 4;
    }
  }

  fseek(input_file, 0, SEEK_SET);

  while(fgets(line, sizeof(line), input_file) != NULL){
    // printf("%s", line);
  }

  // xxd -g 4 file_name
  printf("Wrote %i bytes to %s.\n", bytes_written, output_file_name);

  fclose(input_file);
  fclose(output_file);
  return 0;
}