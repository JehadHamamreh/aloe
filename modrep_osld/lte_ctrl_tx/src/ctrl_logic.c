
#include <oesr.h>

#include "ctrl_logic.h"

/*
 * This function is called before ctrl_params_tslot if, in that timeslot, one of the parameters
 * defined in struct my_parameters has changed. It is this function who needs to check which parameter
 * has changed.
 *
 * \param tslot Value of the current timeslot *
 * \param cur_pm is a pointer to the current parameters, as defined in ctrl_logic.h
 */
int ctrl_params_changed(int tslot, struct my_parameters *cur_pm) {

}

/* This function is called each timeslot, regardless of the parameters having changed or not.
 *
 * \param tslot Value of the current timeslot
 * \param cur_pm is a pointer to the current parameters, as defined in ctrl_logic.h
 */
int ctrl_params_tslot(int tslot, struct my_parameters *cur_pm) {

}
