/*******************************************************************************
 * Copyright (c) 2009-2014, MAV'RIC Development Team
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/*******************************************************************************
 * \file state_default_config.h
 * 
 * \author MAV'RIC Team
 * \author Gregoire Heitz
 *   
 * \brief Default configuration for the state module
 *
 ******************************************************************************/


#ifndef STATE_DEFAULT_CONFIG_H_
#define STATE_DEFAULT_CONFIG_H_

#include "state.hpp"

static inline state_t state_default_config()
{
	state_t conf                 = {};

	conf.mav_mode           	 = MAV_MODE_SAFE;
	conf.mav_state               = MAV_STATE_BOOT;
	conf.simulation_mode         = HIL_OFF;
	conf.autopilot_type          = MAV_TYPE_QUADROTOR;
	conf.autopilot_name          = MAV_AUTOPILOT_MAVRIC;
	conf.sensor_present          = 0b1111110000100111;
	conf.sensor_enabled          = 0b1111110000100111;
	conf.sensor_health           = 0b1111110000100111;
	conf.max_lost_connection     = 60.0f;
	conf.fence_1_xy 	     	 = 100.0f;
	conf.fence_1_z 		     	 = 75.0f;
	conf.fence_2_xy 	     	 = 125.0f;
	conf.fence_2_z 		     	 = 100.0f;

	return conf;
};

#endif // STATE_DEFAULT_CONFIG_H_
