/**
 * \page The MAV'RIC License
 *
 * The MAV'RIC Framework
 *
 * Copyright © 2011-2014
 *
 * Laboratory of Intelligent Systems, EPFL
 */


/**
 * \file onboard_parameters.h
 * 
 * Mav'ric Onboard parameters
 */


#ifndef ONBOARD_PARAMETERS_H_
#define ONBOARD_PARAMETERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mavlink_stream.h"
#include "mavlink_message_handler.h"
#include "scheduler.h"

#include <stdbool.h>

#define MAX_ONBOARD_PARAM_COUNT 120	// should be < 122 to fit on user page on AT32UC3C1512

#define MAVERIC_FLASHC_USER_PAGE_START_ADDRESS AVR32_FLASHC_USER_PAGE_ADDRESS + 0x04	// +4bytes for unknown reason
#define MAVERIC_FLASHC_USER_PAGE_FREE_SPACE 500	// 	512bytes user page, 
												//	-4bytes at the start, 
												//  -8bytes for the protected fuses at the end of the user page
												
/**
 * \brief	Structure of onboard parameter.
 */
typedef struct
{
	float* param;												///< Pointer to the parameter value
	char param_name[MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN];	///< Parameter name composed of 16 characters
	mavlink_message_type_t data_type;							///< Parameter type
	uint8_t param_name_length;									///< Length of the parameter name
	uint8_t param_id;											///< Parameter ID
	bool  schedule_for_transmission;							///< Boolean to activate the transmission of the parameter
} onboard_parameters_entry_t;


/**
 * \brief 		Set of onboard parameters
 * 
 * \details 	Uses C99's flexible member arrays: it is required to 
 * 				allocate memory for this structure
 */
typedef struct
{
	uint32_t param_count;										///< Number of onboard parameter effectively in the array
	uint32_t max_param_count; 									///< Maximum number of parameters
	onboard_parameters_entry_t parameters[];					///< Onboard parameters array, needs memory allocation
} onboard_parameters_set_t;


/**
 * \brief		Main structure of the module onboard parameters
 * 
 * \details  	param_set is implemented as pointer because its memory will be 
 * 				allocated during initialisation
 */
typedef struct 
{
	bool debug;												///< Indicates if debug messages should be printed for each param change
	onboard_parameters_set_t* param_set;					///< Pointer to a set of parameters, needs memory allocation
} onboard_parameters_t;											


/**
 * \brief 	Configuration for the module onboard parameters
 */
typedef struct
{
	uint32_t max_param_count;									///< Maximum number of parameters
	bool debug;													///< Indicates if debug messages should be printed for each param change
} onboard_parameters_conf_t;


/**
 * \brief	TODO: Modify the name of this structure to make it sized as the free flash memory to store these parameters
 */															
typedef struct												
{
	float values[MAVERIC_FLASHC_USER_PAGE_FREE_SPACE];
} nvram_data_t;


/**
* \brief	Initialisation of the Parameter_Set structure by setting the number of onboard parameter to 0
* 
* \param   	onboard_parameters		Pointer to module structure
* \param 	config 					Configuration
* \param 	task_set 				Pointer to mavlink scheduler task set
* \param 	message_handler 		Pointer to mavlink message handler
*/
void onboard_parameters_init(onboard_parameters_t* onboard_parameters, const onboard_parameters_conf_t* config, scheduler_t* scheduler, mavlink_message_handler_t* message_handler);


/**
 * \brief	Register parameter in the internal parameter list that gets published to MAVlink
 *
 * \param   onboard_parameters		Pointer to module structure
 * \param 	val						Unsigned 32 - bits integer parameter value
 * \param 	param_name				Name of the parameter
 */
void onboard_parameters_add_parameter_uint32(onboard_parameters_t* onboard_parameters, uint32_t* val, const char* param_name);


/**
 * \brief	Register parameter in the internal parameter list that gets published to MAVlink
 *
 * \param   onboard_parameters		Pointer to module structure
 * \param 	val						Signed 32 - bits integer parameter value
 * \param 	param_name				Name of the parameter
 */
void onboard_parameters_add_parameter_int32(onboard_parameters_t* onboard_parameters, int32_t* val, const char* param_name);


/**
 * \brief	Registers parameter in the internal parameter list that gets published to MAVlink
 *
 * \param   onboard_parameters		Pointer to module structure
 * \param 	val						Floating point parameter value
 * \param 	param_name				Name of the parameter
 */
void onboard_parameters_add_parameter_float(onboard_parameters_t* onboard_parameters, float* val, const char* param_name);

/**
 * \brief	Read/Write from/to flash depending on the parameters of the mavlink command message
 *
 * \param   onboard_parameters		Pointer to module structure
 * \param   msg 					Incoming mavlink message
 */
void onboard_parameters_preflight_storage(onboard_parameters_t* onboard_parameters, mavlink_command_long_t* msg);

/**
 * \brief	Read onboard parameters from the user page in the flash memory to the RAM memory
 *
 * \param   onboard_parameters		Pointer to module structure
 *
 * \return	The result of the flash read procedure
 */
bool onboard_parameters_read_parameters_from_flashc(onboard_parameters_t* onboard_parameters);


/**
 * \brief	Write onboard parameters to the RAM memory from the user page in the flash memory
 * 
 * \param   onboard_parameters		Pointer to module structure
 */
void onboard_parameters_write_parameters_to_flashc(onboard_parameters_t* onboard_parameters);

#ifdef __cplusplus
}
#endif

#endif /* ONBOARD_PARAMETERS_H */