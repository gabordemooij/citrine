

void ctr_initialize_world();
obj* ctr_find(char* key);
obj* ctr_find_in_my(char* key);
obj* ctr_assign_value(char* name, obj* object);
obj* ctr_assign_value_to_my(char* name, obj* object);
obj* ctr_build_string(char* object);
obj* ctr_build_block(tnode* node);
obj* ctr_build_number(char* object);
obj* ctr_build_bool(int truth);
obj* ctr_build_nil();

obj* ctr_send_message(obj* receiver, char* message, args* argumentList);
char* readf(char* file_name);
void tree(tnode* ti, int indent);
