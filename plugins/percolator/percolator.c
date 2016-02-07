#include "../../citrine.h"

/**
 * Example Plugin 'Coffee Percolator'
 * 
 * to compile this plugin:
 * 
 * gcc -c percolator.c -Wall -Werror -fpic -o percolator.o ; gcc -shared -o libctrpercolator.so percolator.o
 * 
 * Then copy the plugin to the mods folder in the working directory of your script:
 * 
 * mods/percolator/libctrpercolator.so
 * 
 */

/**
 * We use GCC constructor functions to allow plugins
 * to initialize themselves. For other compilers we will have to add a bootstrap function.
 * 
 * I like to call this function 'begin', but you can use any name you like.
 */
void begin (void) __attribute__((constructor));

/**
 * Also note the docblocks above the functions, this documentation format
 * can be parsed by the sman.ctr tool to generate UNIX manual pages.
 */

/**
 * [Percolator] brew
 * 
 * Tries to brew some delicious coffee.
 * To brew one cup of coffee the Percolator needs two cups of water
 * and one spoon of coffee grounds.
 *
 * Read the inline comments to learn how to extend Citrine!
 */
ctr_object* ctr_percolator_brew(ctr_object* myself, ctr_argument* argumentList) {
	
	/**
	 * Fetch the coffee property, note that the key is a Citrine String object.
	 */
	ctr_object* coffee = ctr_internal_object_find_property(
		myself,                        /* owner object */
		ctr_build_string("coffee", 6), /* key object */
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	/**
	 * Fetch the water property.
	 */
	ctr_object* water = ctr_internal_object_find_property(
		myself,
		ctr_build_string("water", 5),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	/**
	 * To access a value:
	 * 
	 * - numeric values reside in value.nvalue
	 * - boolean values reside in value.bvalue
	 * - array   values reside in value.avalue (see ctr_collection)
	 * - string  values reside in value.svalue (see ctr_string)
	 * - block   values reside in value.block  (see ctr_tnode)
	 * - native functions      in value.fvalue
	 */
	if (coffee->value.nvalue < 1) {
		return ctr_build_string("No more coffee.", 16);
	}
	
	if (water->value.nvalue < 2) {
		return ctr_build_string("No more water.", 14);
	}
	
	coffee->value.nvalue -= 1;
	water->value.nvalue -= 2;
	
	/**
	 * To set a property of an object, use
	 * the set_property function.
	 */
	ctr_internal_object_set_property(
		myself, 
		ctr_build_string("coffee", 6),
		coffee,
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	ctr_internal_object_set_property(
		myself, 
		ctr_build_string("water", 5),
		water,
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	return ctr_build_string("Coffee!", 7);
}

/**
 * [Percolator] coffee: [Number] water: [Number]
 * 
 * Adds coffee and water to the perculator.
 * 
 * myPercolator := Percolator new.
 * cupOfCoffee  := myPercolator coffee: 1 water: 2, brew.
 * 
 */
ctr_object* ctr_percolator_add_coffee_water(ctr_object* myself, ctr_argument* argumentList) {
	
	ctr_internal_object_set_property(
		myself, 
		ctr_build_string("coffee", 6),
		ctr_internal_cast2number(argumentList->object),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	ctr_internal_object_set_property(
		myself, 
		ctr_build_string("water", 5),
		ctr_internal_cast2number(argumentList->next->object),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	return myself;	
}

/**
 * Percolator new
 * 
 * Creates a new instance of the percolator object.
 * 
 * Usage:
 * 
 * myPercolator := Percolator new.
 * cupOfCoffee  := myPercolator coffee: 1 water: 2, brew.
 * 
 */
ctr_object* ctr_percolator_new(ctr_object* myself, ctr_argument* argumentList) {
	
	ctr_object* percolatorInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	percolatorInstance->link = myself;
	ctr_internal_object_set_property(
		percolatorInstance, 
		ctr_build_string("coffee", 6),
		ctr_build_number_from_float(0),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	ctr_internal_object_set_property(
		percolatorInstance, 
		ctr_build_string("water", 5),
		ctr_build_number_from_float(0),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	
	return percolatorInstance;	
}

/**
 * C-constructor function, as declared above.
 * 
 * This function gets called when the plugin is loaded into memory.
 * Here you have a chance to add the new object(s) to the World.
 * 
 * In our case, we are going to add the Percolator object to the
 * world.
 */
void begin(){
	/* Create the Coffee Percolator Object - Use new, because its a prototype, not a class !*/
	ctr_object* percolatorObject = ctr_percolator_new(CtrStdObject, NULL);
	
	/* Set the prototype */
	percolatorObject->link = CtrStdObject;

	/* Add the method 'new' so people can create their percolators */
	ctr_internal_create_func(percolatorObject, ctr_build_string("new", 3), &ctr_percolator_new);
	ctr_internal_create_func(percolatorObject, ctr_build_string("brew", 4), &ctr_percolator_brew);
	ctr_internal_create_func(percolatorObject, ctr_build_string("coffee:water:", 13), &ctr_percolator_add_coffee_water);

	/* Make the Percolator accessible to the world */
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Percolator", 10), percolatorObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
