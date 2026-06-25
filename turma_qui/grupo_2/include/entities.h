#ifndef ENTITIES_H
#define ENTITIES_H

#include "types.h"

void entity_list_init(EntityList *list);
void entity_list_free(EntityList *list);
Entity *entity_list_add(EntityList *list, EntityType type, Position pos);
void entity_list_remove_dead(EntityList *list);
const char *entity_type_name(EntityType type);
char entity_symbol(EntityType type);

#endif
