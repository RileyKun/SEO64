#include "class_ids.hpp"

ClassId::ClassId(const hash_t hash) : InstTracker(this)
{
	m_hash = hash;
}

size_t ClassId::get() const
{
	return m_id;
}

bool operator==(const ClassId &a, const size_t b)
{
	return a.m_id == b;
}

void class_ids::init()
{
	if (!i::engine->IsInGame() || !i::engine->IsConnected()) {
		return;
	}

	static bool initialized{};

	if (initialized) {
		return;
	}

	ClientClass *classes{ i::client->GetAllClasses() };

	if (!classes) {
		return;
	}

	for (ClassId *const cid : getInsts<ClassId>())
	{
		for (; classes; classes = classes->m_pNext)
		{
			if (HASH_RT(classes->m_pNetworkName) == cid->m_hash) {
				cid->m_id = classes->m_ClassID;
				break;
			}
		}
	}

	initialized = true;
}