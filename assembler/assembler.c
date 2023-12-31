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

  if(input[0] == '\''){   // Have to escape single-quote :(
    if(input[2] == '\''){
      result = (long) input[1];
      return result;
    }else{
      throw_error("valid single-quote character", input, line_number);
    }
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
  }else if(strcmp(reg, "bp") == 0){
    return 0x4;
  }else if(strcmp(reg, "sp") == 0){
    return 0x5;
  }else{
    // printf("ERROR REGISTER EXPECTED.\n");
    throw_error("valid register", reg, line_number);
    exit(0);
  }
}

void left_shift(char* line, int len, int times){
  int offset = times;
  while(times != 0){
    for(int i = 1; i < len; i++){
      line[i-1] = line[i];
    }
    times--;
  }
  line[strlen(line)-offset] = '\0';
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

int value_is_register(char* val){
    if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0 || strcmp(val, "bp") == 0 || strcmp(val, "sp") == 0){
        return 1;
    }
    return 0;
}

char* get_instruction(char* line){
  char* output = malloc(256);
  strcpy(output, line);

  if(strlen(output) == 0){
    free(output);
    return NULL;
  }

  int space = find_character(output, strlen(output), ' ');
  if(space == -1){
    return output;
  }

  output[space] = '\0';
  return output;

}

char* get_register(char* line){
  char* output = malloc(256);
  strcpy(output, line);

  if(strlen(output) == 0){
    free(output);
    return NULL;
  }


  int space = find_character(output, strlen(output), ' ');
  if(space == -1){
    free(output);
    return NULL;
  }
  space += 1;
  left_shift(output, strlen(output), space);
  int comma = find_character(output, strlen(output), ',');
  int space2 = find_character(output, strlen(output), ' ');
  /*if(comma == -1){
    free(output);
    return NULL;
  }*/

  if(comma == -1 && space2 == -1){
    return output;
  }

  output[comma] = '\0';
  return output;
}

char* get_value(char* line){
  char* output = malloc(256);
  strcpy(output, line);

  if(strlen(output) == 0){
    free(output);
    return NULL;
  }  

  int comma = find_character(output, strlen(output), ',');
  if(comma == -1){
    free(output);
    return NULL;
  }
  comma += 2;
  left_shift(output, strlen(output), comma);
  return output;
}

int assemble_instruction(char* line, int* line_number, unsigned char* machine_code, Label* symbol_table, int* table_size, int* current_address){
  static int temp = 0;
  
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

    char* ins = get_instruction(line_copy);
    char* reg = get_register(line_copy);
    char* val = get_value(line_copy);

    if(ins == NULL && reg == NULL && val == NULL) return 0;
    if(ins == NULL || reg == NULL && strcmp(ins, "ret") && strcmp(ins, "nop") && strcmp(ins, "mul") && strcmp(ins, "div") && strcmp(ins, "hlt") && strcmp(ins, "nop")){
      char err[32];
      snprintf(err, 32, "%s %s, %s", (ins ? ins : ""), (reg ? reg : ""), (val ? val : ""));
      throw_error("valid instruction", err, *line_number);
    }
        
    if(strcmp(ins, "mov") == 0){

      // Implement all 9 mov instructions *:)*

      if(find_character(reg, strlen(reg), '[') != -1 && find_character(val, strlen(val), '[') != -1){
        char err[512];
        sprintf(err, "%s and %s", reg, val);
        throw_error("one pointer", err, *line_number);
      }

      if(find_character(reg, strlen(reg), '[') != -1){
        /*
        Register is pointer. Possible values:
          mov [ax], cx
          mov [ax + 4], cx
          mov [ax + bx], cx
          mov [0x400], cx
        */
        int end = find_character(reg, strlen(reg), ']');
        if(end == -1) throw_error("valid pointer", reg, *line_number);
        
        left_shift(reg, strlen(reg), 1); // Get rid of the '['
        reg[end-1] = '\0'; // Get rid of the ']'
        // [bx + 4] -> bx + 4

        int add = find_character(reg, strlen(reg), '+');
        if(add != -1){
          /*
          Register is pointer by offset. Possible values:
            mov [ax + 4], cx
            mov [ax + bx], cx
          */

          char* ptr_reg = strtok(reg, " + ");
          char* ptr_val = strtok(NULL, " + ");

          if(value_is_register(ptr_val)){
            /*
              mov [ax + bx], cx
            */

            machine_code[0] = 0x07;
            machine_code[1] = register_to_machine_code(ptr_reg);
            machine_code[2] = register_to_machine_code(ptr_val);
            machine_code[3] = register_to_machine_code(val);
          }else{
            /*
              mov [ax + 4], cx
            */

            machine_code[0] = 0x06;
            machine_code[1] = register_to_machine_code(ptr_reg);
            machine_code[2] = convert_to_decimal(ptr_val);
            machine_code[3] = register_to_machine_code(val);
          }

        }else{
          /*
          Register is pointer. Possible values:
            mov [ax], cx
            mov [0x400], cx
          */

          if(value_is_register(reg)){
            /*
              mov [ax], cx
            */

            machine_code[0] = 0x03;
            machine_code[1] = register_to_machine_code(reg);
            machine_code[2] = register_to_machine_code(val);
            machine_code[3] = 0x0;
          }else{
            /*
              mov [0x400], cx
            */
            int decimal = convert_to_decimal(reg);
            machine_code[0] = 0x09;
            machine_code[1] = decimal & 0xFF;
            machine_code[2] = (decimal >> 8) & 0xFF;
            machine_code[3] = register_to_machine_code(val);

          }

        }

      }else if(find_character(val, strlen(val), '[') != -1){
        /*
        Value is pointer. Possible values:
          mov ax, [bx]
          mov ax, [bx + 4]
          mov ax, [bx + cx]
          mov ax, [0x400]
        */

        int end = find_character(val, strlen(val), ']');
        if(end == -1) throw_error("valid pointer", val, *line_number);

        left_shift(val, strlen(val), 1); // Get rid of the '['
        val[end-1] = '\0'; // Get rid of the ']'
        // [0x400] -> 0x400

        int add = find_character(val, strlen(val), '+');
        if(add != -1){
          /*
          Value is pointer by offset. Possible values:
            mov ax, [bx + 4]
            mov ax, [bx + cx]
          */

          char* ptr_reg = strtok(val, " + ");
          char* ptr_val = strtok(NULL, " + ");

          if(value_is_register(ptr_val)){
            /*
              mov ax, [bx + cx]
            */
            machine_code[0] = 0x05;
            machine_code[1] = register_to_machine_code(reg);
            machine_code[2] = register_to_machine_code(ptr_reg);
            machine_code[3] = register_to_machine_code(ptr_val);
          }else{
            /*
              mov ax, [bx + 4]
            */

            machine_code[0] = 0x04;
            machine_code[1] = register_to_machine_code(reg);
            machine_code[2] = register_to_machine_code(ptr_reg);
            machine_code[3] = convert_to_decimal(ptr_val);
          }

        }else{
          /*
          Value is pointer. Possible values:
            mov ax, [bx]
            mov ax, [0x400]
          */

          if(value_is_register(val)){
            /*
              mov ax, [bx]
            */

            machine_code[0] = 0x02;
            machine_code[1] = register_to_machine_code(reg);
            machine_code[2] = register_to_machine_code(val);
            machine_code[3] = 0x0;

          }else{
            /*
              mov ax, [0x400]
            */

            int decimal = convert_to_decimal(val);
            machine_code[0] = 0x08;
            machine_code[1] = register_to_machine_code(reg);
            machine_code[2] = decimal & 0xFF;
            machine_code[3] = (decimal >> 8) & 0xFF;

          }

        }

      }else if(value_is_register(val)){
        /*
          mov ax, bx
        */
        machine_code[0] = 0x01;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = register_to_machine_code(val);
        machine_code[3] = 0x0;
      }else{
        /*
          mov ax, val
        */
        int decimal = convert_to_decimal(val);
        machine_code[0] = 0x00;
        machine_code[1] = register_to_machine_code(reg);
        machine_code[2] = decimal & 0xFF;
        machine_code[3] = (decimal >> 8) & 0xFF;
      }

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

      if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0 || strcmp(val, "bp") == 0 || strcmp(val, "sp") == 0){
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

      if(strcmp(val, "ax") == 0 || strcmp(val, "bx") == 0 || strcmp(val, "cx") == 0 || strcmp(val, "dx") == 0 || strcmp(val, "bp") == 0 || strcmp(val, "sp") == 0){
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
      
      if(strcmp(reg, "ax") == 0 || strcmp(reg, "bx") == 0 || strcmp(reg, "cx") == 0 || strcmp(reg, "dx") == 0 || strcmp(reg, "bp") == 0 || strcmp(reg, "sp") == 0){
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

    if(ins != NULL) free(ins);
    if(reg != NULL) free(reg);
    if(val != NULL) free(val);

    *current_address += 4;
    return 4;
  }
  
  return 0;
}

int main(int argc, char* argv[]){
  if(argc != 3 && argc != 4 && argc != 5){
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

  int q = 0x200;
  int* p = &q;

  store_label("print_char:", symbol_table, &table_size, p);

  q = 0x260;

  store_label("print_str:", symbol_table, &table_size, p);

  // int current_address = 0xE000;
  int current_address;
  int origin_address;
  if(argc == 4 || argc == 5){
    origin_address = convert_to_decimal(argv[3]);
    current_address = convert_to_decimal(argv[3]);
    printf("Code offset set at 0x%X.\n", current_address);

    if(argc == 5 && strcmp(argv[4], "--silent")){
      // printf("Code offset set at 0x%X.\n", current_address);
      printf("***WARNING***\nSetting a different code offset other than the\n");
      printf("recommended one (0xE000) can cause your program to malfunction!!1\n");
      printf("You are running the risk of overwriting core CPU components!!\n");
      printf("Do not proceed if you are simply making a user program.\n\n");
    }

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
