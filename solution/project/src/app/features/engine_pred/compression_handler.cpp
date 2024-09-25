#include "compression_handler.hpp"

void CompressionHandler::onPreEntityPacketReceived(C_BasePlayer *player, int command_slot)
{
	if (command_slot > 0 && player && !player->deadflag()) 
	{
		m_received_packet = true;

		player->RestoreData("", command_slot - 1, PC_EVERYTHING);

		for (auto &vars : m_compressed_vars) {
			vars->onPreEntityPacketReceived(player);
		}
	}
}

void CompressionHandler::onPostNetworkDataReceived(C_BasePlayer *player, int commands_acknowledged)
{
	if (!m_received_packet) {
		return;
	}

	for (auto &vars : m_compressed_vars) {
		vars->onPostNetworkDataReceived(player);
	}
}

// normie: although these variables might generate prediction errors in cl_showerror, it is due to the game thinking that our new values are incorrect (they arent)
// the only detrimental effect is interpolation will get reset but luckily we disabled that...
bool CompressionHandler::onLoad()
{
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(4772, 1.0f / AssignRangeMultiplier(17, 4096.0f - (-4096.0f)), HASH_CT("m_flFallVelocity")));
	m_compressed_vars.emplace_back(std::make_unique<VectorNetvar>(4796, DIST_EPSILON, HASH_CT("m_vecPunchAngle")));
	m_compressed_vars.emplace_back(std::make_unique<VectorNetvar>(4888, DIST_EPSILON, HASH_CT("m_vecPunchAngleVel")));
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(4992, 1.0f / AssignRangeMultiplier(16, 128.f), HASH_CT("m_flStepSize")));

	// view offset gets special treatment because each element is sent with a different range and bit count
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(340, 1.0f / AssignRangeMultiplier(8, 32.0f - (-32.0f)), HASH_CT("m_vecViewOffset_x")));
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(344, 1.0f / AssignRangeMultiplier(8, 32.0f - (-32.0f)), HASH_CT("m_vecViewOffset_y")));
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(348, 1.0f / AssignRangeMultiplier(20, 256.0f), HASH_CT("m_vecViewOffset_z")));

	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(4756, 1.0f / AssignRangeMultiplier(12, 2048.0f), HASH_CT("m_flDuckTime")));
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(4760, 1.0f / AssignRangeMultiplier(12, 2048.0f), HASH_CT("m_flDuckJumpTime")));
	m_compressed_vars.emplace_back(std::make_unique<FloatNetvar>(4764, 1.0f / AssignRangeMultiplier(12, 2048.0f), HASH_CT("m_flJumpTime")));

	return true;
}

bool CompressionHandler::onLevelInit()
{
	m_received_packet = false;

	for (auto &vars : m_compressed_vars) {
		vars->reset();
	}

	return true;
}

bool CompressionHandler::onUnload()
{
	for (auto &vars : m_compressed_vars) {
		vars.release();
	}

	return true;
}

#pragma warning( push )
#pragma warning( disable : 4305 )

float CompressionHandler::AssignRangeMultiplier(int nBits, double range)
{
	unsigned long iHighValue;
	if (nBits == 32)
		iHighValue = 0xFFFFFFFE;
	else
		iHighValue = ((1 << (unsigned long)nBits) - 1);

	float fHighLowMul = iHighValue / range;
	if (CloseEnough(range, 0))
		fHighLowMul = iHighValue;

	// If the precision is messing us up, then adjust it so it won't.
	if ((unsigned long)(fHighLowMul * range) > iHighValue ||
		(fHighLowMul * range) > (double)iHighValue)
	{
		// Squeeze it down smaller and smaller until it's going to produce an integer
		// in the valid range when given the highest value.
		float multipliers[] = { 0.9999, 0.99, 0.9, 0.8, 0.7 };
		int i;
		for (i = 0; i < 4; i++)
		{
			fHighLowMul = (float)(iHighValue / range) * multipliers[i];
			if ((unsigned long)(fHighLowMul * range) > iHighValue ||
				(fHighLowMul * range) > (double)iHighValue)
			{
			}
			else
			{
				break;
			}
		}

		if (i == 4)
		{
			// Doh! We seem to be unable to represent this range.
			return 1.f;
		}
	}

	return fHighLowMul;
}

#pragma warning ( pop )

bool CompressionHandler::CloseEnough(float a, float b, float epsilon)
{
	return fabsf(a - b) <= epsilon;
}

FloatNetvar::FloatNetvar(int offset, float tolerance, hash_t var)
{
	m_offset = offset;
	m_tolerance = tolerance;
	m_var = var;
}

void FloatNetvar::onPreEntityPacketReceived(C_BasePlayer *player)
{
	m_old_value = *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + m_offset);
}

void FloatNetvar::onPostNetworkDataReceived(C_BasePlayer *player)
{
	float new_value{ *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + m_offset) };
	float old_value{ m_old_value };

	if (isInTolerance(&old_value, &new_value)) {
		*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + m_offset) = old_value;
	}
}

bool FloatNetvar::isInTolerance(void *old_value, void *new_value)
{
	float old_val{ *reinterpret_cast<float *>(old_value) };
	float new_val{ *reinterpret_cast<float *>(new_value) };

	if (fabsf(new_val - old_val) <= m_tolerance) {
		return true;
	}

	return false;
}

void FloatNetvar::reset()
{
	m_old_value = {};
}

VectorNetvar::VectorNetvar(int offset, float tolerance, hash_t var)
{
	m_offset = offset;
	m_tolerance = tolerance;
	m_var = var;
}

void VectorNetvar::onPreEntityPacketReceived(C_BasePlayer *player)
{
	m_old_value = *reinterpret_cast<vec3*>(reinterpret_cast<uintptr_t>(player) + m_offset);
}

void VectorNetvar::onPostNetworkDataReceived(C_BasePlayer *player)
{
	vec3 new_value{ *reinterpret_cast<vec3 *>(reinterpret_cast<uintptr_t>(player) + m_offset) };
	vec3 old_value{ m_old_value };

	if (isInTolerance(&old_value, &new_value)) {
		*reinterpret_cast<vec3 *>(reinterpret_cast<uintptr_t>(player) + m_offset) = old_value;
	}
}

bool VectorNetvar::isInTolerance(void *old_value, void *new_value)
{
	const vec3 old_val{ *reinterpret_cast<vec3 *>(old_value) };
	const vec3 new_val{ *reinterpret_cast<vec3 *>(new_value) };

	if (fabsf(new_val.x - old_val.x) <= m_tolerance
		&& fabsf(new_val.y - old_val.y) <= m_tolerance
		&& fabsf(new_val.z - old_val.z) <= m_tolerance) {
		return true;
	}

	return false;
}

void VectorNetvar::reset()
{
	m_old_value = {};
}