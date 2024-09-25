#pragma once

using hash_t = unsigned int;

namespace hash
{
	constexpr unsigned int hash_prime{ 0x1000193u };
	constexpr unsigned int hash_basis{ 0x811C9DC5u };

	template<typename type>
	constexpr bool isUpper(const type character)
	{
		return character >= static_cast<const type>(65) && character <= static_cast<const type>(90);
	}

	template<typename type>
	constexpr type toLower(const type character)
	{
		if (isUpper(character)) {
			return character + static_cast<const type>(32);
		}

		return character;
	}

	template<typename type>
	constexpr bool isTerminator(const type character)
	{
		return character == static_cast<const type>(0);
	}

	template<typename type>
	constexpr size_t getLength(const type *const data)
	{
		size_t length{ 0 };

		while (true)
		{
			if (isTerminator(data[length])) {
				break;
			}

			length++;
		}

		return length;
	}

	template<typename type>
	constexpr hash_t hashCompute(const hash_t hash_basis, const type *data, const size_t size, const bool ignore_case)
	{
		if (size == 0) {
			return hash_basis;
		}

		const hash_t element{ static_cast<hash_t>(ignore_case ? toLower(data[0]) : data[0]) };

		return hashCompute((hash_basis ^ element) * hash_prime, data + 1, size - 1, ignore_case);
	}

	template<typename type>
	constexpr hash_t fnv1a32Hash(const type *const data, const size_t size, const bool ignore_case)
	{
		return hashCompute(hash_basis, data, size, ignore_case);
	}

	constexpr hash_t fnv1a32Hash(const char *const data, const bool ignore_case)
	{
		return fnv1a32Hash(data, getLength(data), ignore_case);
	}

	constexpr hash_t fnv1a32Hash(const wchar_t *const data, const bool ignore_case)
	{
		return fnv1a32Hash(data, getLength(data), ignore_case);
	}
}

#define HASH_CT(data) [&]() { constexpr hash_t hash{ hash::fnv1a32Hash(data, true) }; return hash; }()
#define HASH_RT(data) hash::fnv1a32Hash(data, true)