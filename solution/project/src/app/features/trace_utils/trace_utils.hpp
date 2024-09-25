#pragma once

#include "../../../game/game.hpp"

namespace trace_filters
{
	class FilterWorld final : public ITraceFilter
	{
	public:
		bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask) override;
		TraceType_t GetTraceType() const override;
	};

	class FilterHitscanAim : public ITraceFilter
	{
	public:
		bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask) override;
		TraceType_t GetTraceType() const override;

	public:
		IClientEntity *m_ignore_ent{};
	};

	class FilterMeleeAim : public ITraceFilter
	{
	public:
		bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask) override;
		TraceType_t GetTraceType() const override;

	public:
		IClientEntity *m_ignore_ent{};
	};

	inline FilterWorld world{};
	inline FilterHitscanAim hitscan_aim{};
	inline FilterMeleeAim melee_aim{};
}