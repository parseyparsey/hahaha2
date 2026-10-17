#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

int si;

class AudioSystem {

private:

	static int si;

	ALCdevice* m_device;
	ALCcontext* m_context;

	float m_lastVolume = 1.0f;
	bool m_isMuted = false;

	glm::vec3 m_lastForward;
	glm::vec3 m_lastUp;

	struct SoundSource{
		ALuint buffer;
		ALuint source;
		glm::vec3 position;
		bool isAttachedToObject;
	};

	std::unordered_map<std::string, SoundSource> m_sounds;

	bool loadWAVFile(const std::string& filename, ALuint buffer){
		std::ifstream file(filename, std::ios::binary);
		if(!file.is_open()){
			std::cerr << "Failed to open WAV file!\n";
			return false;
		}

		char chunkID[4];
		file.read(chunkID, 4);
		if (std::string(chunkID, 4) != "RIFF"){
			std::cerr << "Not a Valid WAV File!\n";
			return false;
		}

		file.ignore(4);

		char format[4];
		file.read(format, 4);
		if (std::string(format, 4) != "WAVE"){
			std::cerr << "Not a Valid WAV File!\n";
			return false;
		}

		while(true){
			file.read(chunkID, 4);
			uint32_t chunkSize;
			file.read(reinterpret_cast<char*>(&chunkSize), 4);

			if (std::string(chunkID, 4) == "fmt "){
				uint16_t audioFormat, numChannels, blockAllign, bitsPerSample;
				uint32_t sampleRate, bitRate;

				file.read(reinterpret_cast<char*>(&audioFormat), 2);
				file.read(reinterpret_cast<char*>(&numChannels), 2);
				file.read(reinterpret_cast<char*>(&sampleRate), 4);
				file.read(reinterpret_cast<char*>(&bitRate), 4);
				file.read(reinterpret_cast<char*>(&blockAllign), 2);
				file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

				file.ignore(chunkSize - 16);

				while(true){
					file.read(chunkID, 4);
					file.read(reinterpret_cast<char*>(&chunkSize), 4);

					if (std::string(chunkID, 4) == "data"){
						std::vector<char> data(chunkSize);
						file.read(data.data(), chunkSize);

						ALenum format = AL_FORMAT_MONO16;
						if (numChannels == 1 && bitsPerSample == 8)
							format = AL_FORMAT_MONO8;
						else if (numChannels == 1 && bitsPerSample == 16)
							format = AL_FORMAT_MONO16;
						else if (numChannels == 2 && bitsPerSample == 8)
							format = AL_FORMAT_STEREO8;
						else if (numChannels == 2 && bitsPerSample == 16)
							format = AL_FORMAT_STEREO16;
						else {
							std::cout << "Unsupported WAV Format!\n";
							return false;
						}

						alBufferData(buffer, format, data.data(), data.size(), sampleRate);
						return true;
					} else{
						file.ignore(chunkSize);
					}
				}
			} else {
				file.ignore(chunkSize);
			}
		}

		return true;
	}

public:

	AudioSystem() : m_device(nullptr), m_context(nullptr){};

	~AudioSystem(){
		shutdown();
	}

	void setListenerPos(float x, float y, float z){
			alListener3f(AL_POSITION, x, y, z);
	}

	void setListenerOrien(float fX, float fY, float fZ,
			float uX, float uY, float uZ){
		float orientation[6] = {fX, fY, fZ, uX, uY, uZ};
		alListenerfv(AL_ORIENTATION, orientation);
	}

	bool init(){
		m_device = alcOpenDevice(nullptr);
		if(!m_device){
			std::cerr << "Failed to Open OpenAL Device!\n";
			return false;
		}

		m_context = alcCreateContext(m_device, nullptr);
		if(!m_context){
			std::cerr << "Failed to Create OpenAL Context!\n";
			alcCloseDevice(m_device);
			return false;
		}

		if(!alcMakeContextCurrent(m_context)){
			std::cerr << "Failed to make OpenAL context current" << std::endl;
			alcDestroyContext(m_context);
			alcCloseDevice(m_device);
			return false;
		}

		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

		alDopplerFactor(1.0f);
		alDopplerVelocity(343.0f);

		setListenerPos(0.0f, 0.0f, 0.0f);
		setListenerOrien(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

		std::cout << "AUDIO::OPENAL_SOFT_INIT::OK!\n";
		return true;
	}

	void shutdown(){
		for (auto& [name, sound] : m_sounds){
			alDeleteSources(1, &sound.source);
			alDeleteBuffers(1, &sound.buffer);
		}
		m_sounds.clear();

		if (m_context){
			alcDestroyContext(m_context);
			m_context = nullptr;
		}

		if(m_device){
			alcCloseDevice(m_device);
			m_device = nullptr;
		}
	}

	bool loadSound(const std::string& name, const std::string& filename, bool isLoop,
			const glm::vec3& initialPos = glm::vec3(0.0f)){
		if (m_sounds.find(name) != m_sounds.end()){
			std::cerr << "\"" << name << "\" already exists!\n";
			return false;
		}

		SoundSource sound;
		alGenBuffers(1, &sound.buffer);
		alGenSources(1, &sound.source);

		if(!loadWAVFile(filename, sound.buffer)){
			alDeleteBuffers(1, &sound.buffer);
			alDeleteSources(1, &sound.source);
			return false;
		}

		sound.position = initialPos;
		sound.isAttachedToObject = false;

		alSourcei(sound.source, AL_BUFFER, sound.buffer);
		alSourcef(sound.source, AL_PITCH, 1.0f);
		alSourcef(sound.source, AL_GAIN, 1.0f);
		alSource3f(sound.source, AL_POSITION, initialPos.x, initialPos.y, initialPos.z);
		alSource3f(sound.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

		alSourcef(sound.source, AL_REFERENCE_DISTANCE, 0.2f);
		alSourcef(sound.source, AL_MAX_DISTANCE, 40.0f);
		alSourcef(sound.source, AL_ROLLOFF_FACTOR, 3.0f);
		alSourcei(sound.source, AL_SOURCE_RELATIVE, AL_FALSE);

		if (isLoop)
			alSourcei(sound.source, AL_LOOPING, AL_TRUE);
		else
			alSourcei(sound.source, AL_LOOPING, AL_FALSE);

		m_sounds[name] = sound;
		std::cout << "SOUND" << si << "::" << name << "::" << filename << "::OK!\n";
		si++;
		return true;
	}

	void updateSoundPos(const std::string& name, const glm::vec3& newPos){
		auto it = m_sounds.find(name);
		if (it == m_sounds.end()){
			std::cerr << "\"" << name << "\" not found for position update!\n";
			return;
		}

		it->second.position = newPos;
		alSource3f(it->second.source, AL_POSITION, newPos.x, newPos.y, newPos.z);
	}

	void playSound(const std::string& name, float volume, const
			glm::vec3& position = glm::vec3(0.0f)){
		auto it = m_sounds.find(name);
		if (it == m_sounds.end()){
			std::cerr << "\"" << name << "\" not found!\n";
			return;
		}

		if (position != glm::vec3(0.0f))
			updateSoundPos(name, position);


		alSourcef(it->second.source, AL_GAIN, volume);
		alSourcePlay(it->second.source);
	}

	void stopSound(const std::string& name){
		auto it = m_sounds.find(name);
		if (it != m_sounds.end()){
			alSourceStop(it->second.source);
		}
	}

	bool isSoundPlaying(const std::string& name){
		auto it = m_sounds.find(name);
		if (it != m_sounds.end()){
			ALint state;
			alGetSourcei(it->second.source, AL_SOURCE_STATE, &state);
			return (state == AL_PLAYING);
		}
		return false;
	}

	void setListenerOrienv(const glm::vec3& f, const glm::vec3& up){
		float smoothFactor = 0.05;

		glm::vec3 normf = glm::normalize(f);
		glm::vec3 normup = glm::normalize(up);

		glm::vec3 smoothf = glm::mix(m_lastForward, normf, smoothFactor);
		glm::vec3 smoothup = glm::mix(m_lastUp, normup, smoothFactor);

		smoothf = glm::normalize(smoothf);
		smoothup = glm::normalize(smoothup);

		smoothup = glm::normalize(smoothup - glm::dot(smoothup, smoothf) * smoothf);

		float orientation[6] = {smoothf.x, smoothf.y, smoothf.z,
				smoothup.x, smoothup.y, smoothup.z};
		alListenerfv(AL_ORIENTATION, orientation);

		m_lastForward = smoothf;
		m_lastUp = smoothup;
	}

	void confSoundDistance(const std::string& name, float refDistance,
			float maxDistance, float rollofFactor){
		auto it = m_sounds.find(name);
		if (it != m_sounds.end()){
			alSourcef(it->second.source, AL_REFERENCE_DISTANCE, refDistance);
			alSourcef(it->second.source, AL_MAX_DISTANCE, maxDistance);
			alSourcef(it->second.source, AL_ROLLOFF_FACTOR, rollofFactor);
		}
	}

	void setDistanceModel(const std::string& model){
		if (model == "linear")
			alDistanceModel(AL_LINEAR_DISTANCE);
		else if (model == "exponential")
			alDistanceModel(AL_EXPONENT_DISTANCE);
		else if (model == "inverse")
			alDistanceModel(AL_INVERSE_DISTANCE);
		else if (model == "linear_clamped")
			alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		else if (model == "exponential_clamped")
			alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
		else if (model == "inverse_clamped")
			alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	}

	void setMasterVolume(float volume){
		if (!m_isMuted)
			alListenerf(AL_GAIN, volume);

		m_lastVolume = volume;
	}

	void mute(bool isMuted){
		if (isMuted){
			alGetListenerf(AL_GAIN, &m_lastVolume);
			alListenerf(AL_GAIN, 0.0f);
			m_isMuted = true;
		} else {
			alListenerf(AL_GAIN, m_lastVolume);
			m_isMuted = false;
		}
	}

};

#endif
