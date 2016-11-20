
#ifndef _GAME_VARIABLES_H_
#define _GAME_VARIABLES_H_

#define GAME_NAME "Tremor"

typedef struct _LevelStats
{
    BOOL locked;
	BOOL scored;

	I32 score;
	float time;

} LevelStats;

extern LevelStats level_stats[LEVELS_COUNT];

extern float global_fade_k;

extern float sound_volume;

#endif /* _GAME_VARIABLES_H_ */

