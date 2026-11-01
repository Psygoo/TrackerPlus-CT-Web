#ifndef S13B_LEVELS_H
#define S13B_LEVELS_H

/* Change only these ratios for later hardware calibration.  The renderer
 * converts the requested PWM fraction back to the nearest stock 0..31 gamma
 * level under the user's current bright-pad ceiling. */
#define S13B_ACTIVE_MEMBER_NUM 32u
#define S13B_ACTIVE_MEMBER_DEN 85u
#define S13B_OTHER_GROUP_NUM  11u
#define S13B_OTHER_GROUP_DEN  85u
#define S13B_UNASSIGNED_NUM    2u
#define S13B_UNASSIGNED_DEN   51u

#endif
