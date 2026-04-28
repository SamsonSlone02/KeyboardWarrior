#include "GameSound.h"

GameSound::GameSound()
	: initialized_(false)
{
}

GameSound::~GameSound()
{
	shutdown();
}

bool GameSound::init()
{
	if (initialized_)
		return true;

	clearError();
	alutInit(0, NULL);
	if (alGetError() != AL_NO_ERROR) {
		setError("alutInit failed");
		return false;
	}

	alGetError();
	alListenerf(AL_GAIN, 1.0f);
	initialized_ = (alGetError() == AL_NO_ERROR);
	if (!initialized_) {
		setError("OpenAL setup failed");
		alutExit();
	}
	return initialized_;
}

void GameSound::shutdown()
{
	if (!initialized_ && sounds_.empty())
		return;

	for (size_t i = 0; i < sounds_.size(); ++i) {
		alDeleteSources(1, &sounds_[i].source);
		alDeleteBuffers(1, &sounds_[i].buffer);
	}
	sounds_.clear();

	if (initialized_) {
		alutExit();
		initialized_ = false;
	}
}

int GameSound::loadWav(const std::string &filename, bool looping,
	float gain, float pitch)
{
	if (!initialized_) {
		setError("sound system not initialized");
		return -1;
	}

	clearError();
	ALuint buffer = alutCreateBufferFromFile(filename.c_str());
	if (buffer == AL_NONE || alGetError() != AL_NO_ERROR) {
		setError("failed to load wav file");
		return -1;
	}

	ALuint source = 0;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	alSourcef(source, AL_GAIN, gain);
	alSourcef(source, AL_PITCH, pitch);

	if (alGetError() != AL_NO_ERROR) {
		alDeleteSources(1, &source);
		alDeleteBuffers(1, &buffer);
		setError("failed to create sound source");
		return -1;
	}

	Sound sound;
	sound.buffer = buffer;
	sound.source = source;
	sounds_.push_back(sound);
	return static_cast<int>(sounds_.size() - 1);
}

bool GameSound::play(int soundId)
{
	if (!validSoundId(soundId))
		return false;
	alSourcePlay(sounds_[soundId].source);
	return (alGetError() == AL_NO_ERROR);
}

void GameSound::stop(int soundId)
{
	if (!validSoundId(soundId))
		return;
	alSourceStop(sounds_[soundId].source);
}

void GameSound::setLoop(int soundId, bool looping)
{
	if (!validSoundId(soundId))
		return;
	alSourcei(sounds_[soundId].source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
}

void GameSound::setGain(int soundId, float gain)
{
	if (!validSoundId(soundId))
		return;
	alSourcef(sounds_[soundId].source, AL_GAIN, gain);
}

void GameSound::setPitch(int soundId, float pitch)
{
	if (!validSoundId(soundId))
		return;
	alSourcef(sounds_[soundId].source, AL_PITCH, pitch);
}

bool GameSound::isReady() const
{
	return initialized_;
}

const char *GameSound::lastError() const
{
	return lastError_.c_str();
}

bool GameSound::validSoundId(int soundId) const
{
	return soundId >= 0 && soundId < static_cast<int>(sounds_.size());
}

void GameSound::setError(const char *message)
{
	lastError_ = message;
}

void GameSound::clearError()
{
	lastError_.clear();
}
