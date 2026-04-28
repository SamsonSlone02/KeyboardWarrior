#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <string>
#include <vector>

#include <AL/alut.h>

class GameSound {
public:
	GameSound();
	~GameSound();

	GameSound(const GameSound &) = delete;
	GameSound &operator=(const GameSound &) = delete;

	bool init();
	void shutdown();

	int loadWav(const std::string &filename, bool looping = false,
		float gain = 1.0f, float pitch = 1.0f);
	bool play(int soundId);
	void stop(int soundId);
	void setLoop(int soundId, bool looping);
	void setGain(int soundId, float gain);
	void setPitch(int soundId, float pitch);

	bool isReady() const;
	const char *lastError() const;

private:
	struct Sound {
		ALuint buffer;
		ALuint source;
	};

	bool validSoundId(int soundId) const;
	void setError(const char *message);
	void clearError();

	std::vector<Sound> sounds_;
	bool initialized_;
	std::string lastError_;
};

#endif
