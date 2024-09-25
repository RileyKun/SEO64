#pragma once

#include "../../../game/game.hpp"

class BaseNetvar
{
public:
	int m_offset{};
	float m_tolerance{};
	hash_t m_var{};

public:
	virtual void onPreEntityPacketReceived(C_BasePlayer *player) = 0;
	virtual void onPostNetworkDataReceived(C_BasePlayer *player) = 0;
	virtual bool isInTolerance(void *old_value, void *new_value) = 0;
	virtual void reset() = 0;
};

class FloatNetvar final : public BaseNetvar
{
public:
	float m_old_value{};

public:
	FloatNetvar(int offset, float tolerance, hash_t var);

public:
	void onPreEntityPacketReceived(C_BasePlayer *player) override;
	void onPostNetworkDataReceived(C_BasePlayer *player) override;
	bool isInTolerance(void *old_value, void *new_value) override;
	void reset() override;
};

class VectorNetvar final : public BaseNetvar
{
public:
	vec3 m_old_value{};

public:
	VectorNetvar(int offset, float tolerance, hash_t var);

public:
	void onPreEntityPacketReceived(C_BasePlayer *player) override;
	void onPostNetworkDataReceived(C_BasePlayer *player) override;
	bool isInTolerance(void *old_value, void *new_value) override;
	void reset() override;
};

class CompressionHandler final : public HasLoad, public HasLevelInit, public HasUnload
{
private:
	std::vector<std::unique_ptr<BaseNetvar>> m_compressed_vars{};

private:
	bool m_received_packet{};

public:
	void onPreEntityPacketReceived(C_BasePlayer *player, int command_slot);
	void onPostNetworkDataReceived(C_BasePlayer *player, int commands_acknowledged);

public:
	bool onLoad() override;
	bool onLevelInit() override;
	bool onUnload() override;

private:
	float AssignRangeMultiplier(int nBits, double range);
public:
	bool CloseEnough(float a, float b, float epsilon = EQUAL_EPSILON);
};

MAKE_UNIQUE(CompressionHandler, compression_handler)