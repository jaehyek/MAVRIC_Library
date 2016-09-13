/*******************************************************************************
 * Copyright (c) 2009-2016, MAV'RIC Development Team
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
 * \file mission_handler_critical_navigating.hxx
 *
 * \author MAV'RIC Team
 * \author Matthew Douglas
 *
 * \brief   The MAVLink mission planner handler functions for the critical 
 *          navigating state
 *
 ******************************************************************************/

 
#ifndef MISSION_HANDLER_CRITICAL_NAVIGATING_HXX__
#define MISSION_HANDLER_CRITICAL_NAVIGATING_HXX__

#include "communication/mavlink_waypoint_handler.hpp"
#include "mission/mission_handler_navigating.hpp"

template <class T>
Mission_handler_critical_navigating<T>::Mission_handler_critical_navigating(T& controller,
                                                                            const INS& ins,
                                                                            Navigation& navigation,
                                                                            const Mavlink_stream& mavlink_stream,
                                                                            Mavlink_waypoint_handler& waypoint_handler):
            Mission_handler_navigating<T>(controller, ins, navigation, mavlink_stream, waypoint_handler)
{
}

template <class T>
bool Mission_handler_critical_navigating<T>::can_handle(const Waypoint& wpt) const
{
    bool handleable = false;

    uint16_t cmd = wpt.command();
    if (cmd == MAV_CMD_NAV_CRITICAL_WAYPOINT)
    {
        handleable = true;
    }

    return handleable;
}

#endif // MISSION_HANDLER_CRITICAL_NAVIGATING_HXX__
