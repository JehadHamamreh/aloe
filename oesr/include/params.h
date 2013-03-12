/*
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 *
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Note: The parameter type must coincide (the order) with oesr_var_type_t in oesr_types.h */

/** Parameter type */
typedef enum {INT, FLOAT, STRING} param_type_t;

typedef void* pmid_t;


/** Stores in ptr up to max_size elements of the value of the parameter with id the value
 * returned by the function param_id().
 *
 * Sets the number of written elements to the pointer size and the parameter type to "type".
 *
 * The following format for parameter values is assumed:
 *    - *.app file (vectors are comma-separted values):
 *    	- float: Number with a decimal point "."
 *    	- int: Number with no decimal point or hexadecimal number (begining with 0x)
 *    	- string: Character string
 *
 *    - standalone arguments (vector values not yet supported):
 *    	- float: Number with a decimal point "."
 *    	- int: Number with no decimal point or hexadecimal number (begining with 0x)
 *    	- string: Anything else
 *
 *    - Matlab/Octave (vector values not yet supported):
 *    	- float: Default matlab/octave numeric value
 *    	- int: Matlab/octave int32 type (e.g. a=int32(x))
 *    	- string: Not supported
 *
 *  \returns -1 on error or a non-negative integer indicating the number of written bytes
 *  on success
 */
int param_get(pmid_t id, void *ptr, int max_size, param_type_t *type);


/** Returns a positive integer identifying the parameter name. The functions
 * param_get_int_id() and param_get_float_id() can then be used as the functions
 * param_get_int() or param_get_float() to obtain faster access to the parameter values.
 * Useful if the module needs to obtain the updated values continuously.
 */
pmid_t param_id(char *name);

/* Sets the value of a remote parameter assuming that the control plane is configured automatically
 * (field auto_ctrl_module in the app).
 *
 * This mode assumes that the i-th output interface of the control module is connected with
 * the control interface of the i-th module of the waveform.
 *
 * \param out_ptr is the set of output interfaces pointers given in the work() function of the control module
 * \param module_idx Index in the waveform of the remote module
 * \param param_idx Index of the variable to change in the remote module's parameters set
 * \param value Pointer to the new variable data
 * \param value_sz size of the new data (in bytes)
 */
int param_remote_set(void **out_ptr, int module_idx, int param_idx, void *value, int value_sz);

/**
 * @returns -1 on error, 0 if parameter found but not integer, 1 on success
 */
int param_get_int(pmid_t id, int *value);
/**
 * @returns -1 on error, 0 if parameter found but not integer, 1 on success
 */
int param_get_float(pmid_t id, float *value);
/**
 * @returns -1 on error, 0 on success
 */
int param_get_int_name(char *name, int *value);
/**
 * @returns -1 on error, 0 on success
 */
int param_get_float_name(char *name, float *value);


struct utils_variables {
	char *name;
	int size;
	void *value;
};

typedef struct utils_variables user_var_t;



/** Parameter object */
typedef struct {
	char *name;
	param_type_t type;
	int size;
}param_t;
