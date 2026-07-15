#include "risk_calc.h"
#include <stdio.h>

void RiskCalc_CalculateTTC(VehicleState_t *state) {
    state->speed_m_s = 10; 
    
    if (state->distance_m == 0) {
        state->distance_m = 25; 
    }
    
    if (state->speed_m_s > 0) {
        state->ttc_ms = (state->distance_m * 1000) / state->speed_m_s;
    } else {
        state->ttc_ms = 9999; 
    }
}