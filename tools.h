

void dnk_initialize_world();
obj* dnk_find(char* key);
obj* dnk_find_in_my(char* key);
obj* dnk_assign_value(char* name, obj* object);
obj* dnk_assign_value_to_my(char* name, obj* object);
obj* dnk_build_string(char* object);
obj* dnk_build_block(tnode* node);
obj* dnk_build_number(char* object);
obj* dnk_build_bool(int truth);
obj* dnk_build_nil();

obj* dnk_send_message(obj* receiver, char* message, args* argumentList);
char* readf(char* file_name);
void tree(tnode* ti, int indent);
