// UserSounds.h
enum	{S_DoorClose, S_DoorOpen, S_Fader, S_DoorChime, S_NbrSounds};

void InitMusic(void);

void PlayTune(char *s);
void EndMusic(void);