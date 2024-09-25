#pragma once
#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cmath>

namespace owoify {

	class Utility;

	class Word
	{
	public:
		explicit Word(std::wstring word)
			: m_word(std::move(word)), m_replacedWords(std::unordered_set<std::wstring>())
		{

		}

		Word(const Word &word)
			: m_word(word.m_word), m_replacedWords(word.m_replacedWords)
		{

		}

		Word(Word &&word) noexcept
			: m_word(std::move(word.m_word)), m_replacedWords(std::move(word.m_replacedWords))
		{

		}

		Word &operator=(const Word &word)
		{
			if (&word == this) return *this;

			m_word = word.m_word;
			m_replacedWords = word.m_replacedWords;
			return *this;
		}

		Word &operator=(Word &&word) noexcept
		{
			if (&word == this) return *this;

			m_word = std::move(word.m_word);
			m_replacedWords = std::move(word.m_replacedWords);
			return *this;
		}

		friend std::wstringstream &operator<<(std::wstringstream &out, const Word &word);
		friend class Utility;

	private:
		Word &replace(const std::wregex &regex, const std::wstring &replaceValue,
					  bool replaceReplacedWords = false)
		{
			if (!replaceReplacedWords &&
				searchValueContainsReplacedWords(regex, replaceValue))
				return *this;

			auto replacing_word = m_word;

			if (std::regex_search(m_word, regex))
				replacing_word = std::regex_replace(m_word, regex, replaceValue);

			auto begin = std::wsregex_iterator(m_word.cbegin(), m_word.cend(), regex);
			auto end = std::wsregex_iterator();
			auto replaced_words = std::vector<std::wstring>();

			for (auto i = begin; i != end; ++i)
			{
				replaced_words.emplace_back(std::regex_replace(i->str(), regex, replaceValue));
			}

			if (replacing_word != m_word)
			{
				for (const auto &word : replaced_words)
				{
					m_replacedWords.emplace(word);
				}
				m_word = replacing_word;
			}

			return *this;
		}

		Word &replace(const std::wregex &regex, const std::function<std::wstring()> &func,
					  bool replaceReplacedWords = false)
		{
			auto replace_value = func();

			if (!replaceReplacedWords &&
				searchValueContainsReplacedWords(regex, replace_value))
				return *this;

			auto replacing_word = m_word;

			if (std::regex_search(m_word, regex))
				replacing_word = std::regex_replace(m_word, regex, replace_value);

			auto begin = std::wsregex_iterator(m_word.cbegin(), m_word.cend(), regex);
			auto end = std::wsregex_iterator();
			auto replaced_words = std::vector<std::wstring>();

			for (auto i = begin; i != end; ++i)
			{
				replaced_words.emplace_back(std::regex_replace(i->str(), regex, replace_value));
			}

			if (replacing_word != m_word)
			{
				for (const auto &word : replaced_words)
				{
					m_replacedWords.emplace(word);
				}
				m_word = replacing_word;
			}

			return *this;
		}

		Word &replace(const std::wregex &regex,
					  const std::function<std::wstring(const std::wstring &, const std::wstring &)> func,
					  bool replaceReplacedWords = false)
		{
			if (!std::regex_search(m_word, regex))
				return *this;

			auto match = std::wsmatch();
			std::regex_match(m_word, match, regex);
			auto replace_value = func(match[1], match[2]);

			if (!replaceReplacedWords &&
				searchValueContainsReplacedWords(regex, replace_value))
				return *this;

			auto replacing_word = m_word;

			if (std::regex_search(m_word, regex))
				replacing_word = std::regex_replace(m_word, regex, replace_value);

			auto begin = std::wsregex_iterator(m_word.cbegin(), m_word.cend(), regex);
			auto end = std::wsregex_iterator();
			auto replaced_words = std::vector<std::wstring>();

			for (auto i = begin; i != end; ++i)
			{
				replaced_words.emplace_back(std::regex_replace(i->str(), regex, replace_value));
			}

			if (replacing_word != m_word)
			{
				for (const auto &word : replaced_words)
				{
					m_replacedWords.emplace(word);
				}
				m_word = replacing_word;
			}

			return *this;
		}

	private:
		bool searchValueContainsReplacedWords(const std::wregex &regex, const std::wstring &replaceValue)
		{
			auto result = std::any_of(m_replacedWords.cbegin(), m_replacedWords.cend(), [&](const std::wstring &word) {
				if (std::regex_search(word, regex))
				{
					return std::regex_replace(word, regex, replaceValue) == word;
				}
				return false;
									  });

			return result;
		}

		std::wstring m_word;
		std::unordered_set<std::wstring> m_replacedWords;
	};

	class Owoifier;

	class Utility
	{
	public:
		friend class Owoifier;

		template <typename T = std::wstring>
		static std::deque<T> interleaveArrays(const std::deque<T> &a, const std::deque<T> &b)
		{
			auto arr = std::deque<T>();
			auto observed = a;
			auto other = b;

			while (observed.size() > 0)
			{
				arr.emplace_back(observed[0]);
				observed.pop_front();

				std::swap(observed, other);
			}

			if (other.size() > 0)
			{
				std::move(other.begin(), other.end(), std::back_inserter(arr));
			}

			return arr;
		}

	private:
		inline static const std::wregex MAP_O_TO_OWO = std::basic_regex(L"o", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_EW_TO_UWU = std::basic_regex(L"ew", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_HEY_TO_HAY = std::basic_regex(L"([Hh])ey", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_DEAD_TO_DED_UPPER = std::basic_regex(L"Dead", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_DEAD_TO_DED_LOWER = std::basic_regex(L"dead", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_N_VOWEL_T_TO_ND = std::basic_regex(L"n[aeiou]*t", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_READ_TO_WEAD_UPPER = std::basic_regex(L"Read", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_READ_TO_WEAD_LOWER = std::basic_regex(L"read", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_BRACKETS_TO_STARTRAILS_FORE = std::basic_regex(L"[({<]", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_BRACKETS_TO_STARTRAILS_REAR = std::basic_regex(L"[)}>]", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_PERIOD_COMMA_EXCLAMATION_SEMICOLON_TO_KAOMOJIS_FIRST = std::basic_regex(L"[.,](?![0-9])", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_PERIOD_COMMA_EXCLAMATION_SEMICOLON_TO_KAOMOJIS_SECOND = std::basic_regex(L"[!;]+", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_THAT_TO_DAT_LOWER = std::basic_regex(L"that", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_THAT_TO_DAT_UPPER = std::basic_regex(L"That", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_TH_TO_F_UPPER = std::basic_regex(L"TH(?!E)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_TH_TO_F_LOWER = std::basic_regex(L"[Tt]h(?![Ee])", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_LE_TO_WAL = std::basic_regex(L"le$", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_VE_TO_WE_LOWER = std::basic_regex(L"ve", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_VE_TO_WE_UPPER = std::basic_regex(L"Ve", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_RY_TO_WWY = std::basic_regex(L"ry", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_RORL_TO_W_LOWER = std::basic_regex(L"(?:r|l)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_RORL_TO_W_UPPER = std::basic_regex(L"(?:R|L)", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_LL_TO_WW = std::basic_regex(L"ll", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_VOWEL_OR_R_EXCEPT_O_L_TO_WL_LOWER = std::basic_regex(L"[aeiur]l$", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_VOWEL_OR_R_EXCEPT_O_L_TO_WL_UPPER = std::basic_regex(L"[AEIUR]([lL])$", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_OLD_TO_OWLD_LOWER = std::basic_regex(L"([Oo])ld", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_OLD_TO_OWLD_UPPER = std::basic_regex(L"OLD", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_OL_TO_OWL_LOWER = std::basic_regex(L"([Oo])l", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_OL_TO_OWL_UPPER = std::basic_regex(L"OL", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_LORR_O_TO_WO_LOWER = std::basic_regex(L"[lr]o", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_LORR_O_TO_WO_UPPER = std::basic_regex(L"[LR]([oO])", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_SPECIFIC_CONSONANTS_O_TO_LETTER_AND_WO_LOWER = std::basic_regex(L"([bcdfghjkmnpqstxyz])o", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_SPECIFIC_CONSONANTS_O_TO_LETTER_AND_WO_UPPER = std::basic_regex(L"([BCDFGHJKMNPQSTXYZ])([oO])", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_VORW_LE_TO_WAL = std::basic_regex(L"[vw]le", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_FI_TO_FWI_LOWER = std::basic_regex(L"([Ff])i", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_FI_TO_FWI_UPPER = std::basic_regex(L"FI", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_VER_TO_WER = std::basic_regex(L"([Vv])er", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_POI_TO_PWOI = std::basic_regex(L"([Pp])oi", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_SPECIFIC_CONSONANTS_LE_TO_LETTER_AND_WAL = std::basic_regex(L"([DdFfGgHhJjPpQqRrSsTtXxYyZz])le$", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_CONSONANT_R_TO_CONSONANT_W = std::basic_regex(L"([BbCcDdFfGgKkPpQqSsTtWwXxZz])r", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_LY_TO_WY_LOWER = std::basic_regex(L"ly", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_LY_TO_WY_UPPER = std::basic_regex(L"Ly", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_PLE_TO_PWE = std::basic_regex(L"([Pp])le", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_NR_TO_NW_LOWER = std::basic_regex(L"nr", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_NR_TO_NW_UPPER = std::basic_regex(L"NR", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_FUC_TO_FWUC = std::basic_regex(L"([Ff])uc", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_MOM_TO_MWOM = std::basic_regex(L"([Mm])om", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_ME_TO_MWE = std::basic_regex(L"([Mm])e", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_N_VOWEL_TO_NY_FIRST = std::basic_regex(L"n([aeiou])", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_N_VOWEL_TO_NY_SECOND = std::basic_regex(L"N([aeiou])", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_N_VOWEL_TO_NY_THIRD = std::basic_regex(L"N([AEIOU])", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_OVE_TO_UV_LOWER = std::basic_regex(L"ove", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_OVE_TO_UV_UPPER = std::basic_regex(L"OVE", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_HAHA_TO_HEHE_XD = std::basic_regex(L"\\b(ha|hah|heh|hehe)+\\b", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_THE_TO_TEH = std::basic_regex(L"\\b([Tt])he\\b", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_YOU_TO_U_UPPER = std::basic_regex(L"\\bYou\\b", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_YOU_TO_U_LOWER = std::basic_regex(L"\\byou\\b", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		inline static const std::wregex MAP_TIME_TO_TIM = std::basic_regex(L"\\b([Tt])ime\\b", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_OVER_TO_OWOR = std::basic_regex(L"([Oo])ver", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		inline static const std::wregex MAP_WORSE_TO_WOSE = std::basic_regex(L"([Ww])orse", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		static Word map_O_To_OwO(Word &input)
		{
			auto number = ::roundf(getRandomValue(0.0f, 1.0f));
			return input.replace(MAP_O_TO_OWO, number > 0.0f ? L"owo" : L"o");
		}

		static Word map_Ew_To_UwU(Word &input)
		{
			return input.replace(MAP_EW_TO_UWU, L"uwu");
		}

		static Word map_Hey_To_Hay(Word &input)
		{
			return input.replace(MAP_HEY_TO_HAY, L"$1ay");
		}

		static Word map_Dead_To_Ded(Word &input)
		{
			return input.replace(MAP_DEAD_TO_DED_UPPER, L"Ded")
				.replace(MAP_DEAD_TO_DED_LOWER, L"ded");
		}

		static Word map_N_Vowel_T_To_Nd(Word &input)
		{
			return input.replace(MAP_N_VOWEL_T_TO_ND, L"nd");
		}

		static Word map_Read_To_Wead(Word &input)
		{
			return input.replace(MAP_READ_TO_WEAD_UPPER, L"Wead")
				.replace(MAP_READ_TO_WEAD_LOWER, L"wead");
		}

		static Word map_Brackets_To_StarTrails(Word &input)
		{
			return input.replace(MAP_BRACKETS_TO_STARTRAILS_FORE, L"｡･:*:･ﾟ★,｡･:*:･ﾟ☆")
				.replace(MAP_BRACKETS_TO_STARTRAILS_REAR, L"☆ﾟ･:*:･｡,★ﾟ･:*:･｡");
		}

		static Word map_PeriodCommaExclamationSemicolon_To_Kaomojis(Word &input)
		{
			auto index = ::floorf(getRandomValue(0.0f, 1.0f) * m_faces.size());
			input = input.replace(MAP_PERIOD_COMMA_EXCLAMATION_SEMICOLON_TO_KAOMOJIS_FIRST, [&]() {
				return L" " + m_faces[static_cast<size_t>(index)];
								  });

			index = ::floorf(getRandomValue(0.0f, 1.0f) * m_faces.size());
			input = input.replace(MAP_PERIOD_COMMA_EXCLAMATION_SEMICOLON_TO_KAOMOJIS_SECOND, [&]() {
				return L" " + m_faces[static_cast<size_t>(index)];
								  });

			return input;
		}

		static Word map_That_To_Dat(Word &input)
		{
			return input.replace(MAP_THAT_TO_DAT_LOWER, L"dat")
				.replace(MAP_THAT_TO_DAT_UPPER, L"Dat");
		}

		static Word map_Th_To_F(Word &input)
		{
			return input.replace(MAP_TH_TO_F_LOWER, L"f")
				.replace(MAP_TH_TO_F_UPPER, L"F");
		}

		static Word map_Le_To_Wal(Word &input)
		{
			return input.replace(MAP_LE_TO_WAL, L"wal");
		}

		static Word map_Ve_To_We(Word &input)
		{
			return input.replace(MAP_VE_TO_WE_LOWER, L"we")
				.replace(MAP_VE_TO_WE_UPPER, L"We");
		}

		static Word map_Ry_To_Wwy(Word &input)
		{
			return input.replace(MAP_RY_TO_WWY, L"wwy");
		}

		static Word map_ROrL_To_W(Word &input)
		{
			return input.replace(MAP_RORL_TO_W_LOWER, L"w")
				.replace(MAP_RORL_TO_W_UPPER, L"W");
		}

		static Word map_Ll_To_Ww(Word &input)
		{
			return input.replace(MAP_LL_TO_WW, L"ww");
		}

		static Word map_VowelOrRExceptO_L_To_Wl(Word &input)
		{
			return input.replace(MAP_VOWEL_OR_R_EXCEPT_O_L_TO_WL_LOWER, L"wl")
				.replace(MAP_VOWEL_OR_R_EXCEPT_O_L_TO_WL_UPPER, L"W$1");
		}

		static Word map_Old_To_Owld(Word &input)
		{
			return input.replace(MAP_OLD_TO_OWLD_LOWER, L"$1wld")
				.replace(MAP_OLD_TO_OWLD_UPPER, L"OWLD");
		}

		static Word map_Ol_To_Owl(Word &input)
		{
			return input.replace(MAP_OL_TO_OWL_LOWER, L"$1wl")
				.replace(MAP_OL_TO_OWL_UPPER, L"OWL");
		}

		static Word map_LOrR_O_To_Wo(Word &input)
		{
			return input.replace(MAP_LORR_O_TO_WO_LOWER, L"wo")
				.replace(MAP_LORR_O_TO_WO_UPPER, L"W$1");
		}

		static Word map_SpecificConsonants_O_To_Letter_And_Wo(Word &input)
		{
			return input.replace(MAP_SPECIFIC_CONSONANTS_O_TO_LETTER_AND_WO_LOWER, L"$1wo")
				.replace(MAP_SPECIFIC_CONSONANTS_O_TO_LETTER_AND_WO_UPPER, [&](const std::wstring &m1,
						 const std::wstring &m2) {

							 std::wstring upper = L"";
							 for (const auto &c : m2)
							 {
								 upper += c;
							 }

							 return m1 + (upper == m2 ? L"W" : L"w") + m2;
						 });
		}

		static Word map_VOrW_Le_To_Wal(Word &input)
		{
			return input.replace(MAP_VORW_LE_TO_WAL, L"wal");
		}

		static Word map_Fi_To_Fwi(Word &input)
		{
			return input.replace(MAP_FI_TO_FWI_LOWER, L"$1wi")
				.replace(MAP_FI_TO_FWI_UPPER, L"FWI");
		}

		static Word map_Ver_To_Wer(Word &input)
		{
			return input.replace(MAP_VER_TO_WER, L"wer");
		}

		static Word map_Poi_To_Pwoi(Word &input)
		{
			return input.replace(MAP_POI_TO_PWOI, L"$1woi");
		}

		static Word map_SpecificConsonants_Le_To_Letter_And_Wal(Word &input)
		{
			return input.replace(MAP_SPECIFIC_CONSONANTS_LE_TO_LETTER_AND_WAL, L"$1wal");
		}

		static Word map_Consonant_R_To_Consonant_W(Word &input)
		{
			return input.replace(MAP_CONSONANT_R_TO_CONSONANT_W, L"$1w");
		}

		static Word map_Ly_To_Wy(Word &input)
		{
			return input.replace(MAP_LY_TO_WY_LOWER, L"wy")
				.replace(MAP_LY_TO_WY_UPPER, L"Wy");
		}

		static Word map_Ple_To_Pwe(Word &input)
		{
			return input.replace(MAP_PLE_TO_PWE, L"1we");
		}

		static Word map_Nr_To_Nw(Word &input)
		{
			return input.replace(MAP_NR_TO_NW_LOWER, L"nw")
				.replace(MAP_NR_TO_NW_UPPER, L"NW");
		}

		static Word map_Fuc_To_Fwuc(Word &input)
		{
			return input.replace(MAP_FUC_TO_FWUC, L"$1wuc");
		}

		static Word map_Mom_To_Mwom(Word &input)
		{
			return input.replace(MAP_MOM_TO_MWOM, L"$1wom");
		}

		static Word map_Me_To_Mwe(Word &input)
		{
			return input.replace(MAP_ME_TO_MWE, L"$1we");
		}

		static Word map_NVowel_To_Ny(Word &input)
		{
			return input.replace(MAP_N_VOWEL_TO_NY_FIRST, L"ny$1")
				.replace(MAP_N_VOWEL_TO_NY_SECOND, L"Ny$1")
				.replace(MAP_N_VOWEL_TO_NY_THIRD, L"NY$1");
		}

		static Word map_Ove_To_Uv(Word &input)
		{
			return input.replace(MAP_OVE_TO_UV_LOWER, L"uv")
				.replace(MAP_OVE_TO_UV_UPPER, L"UV");
		}

		static Word map_Haha_To_HehexD(Word &input)
		{
			return input.replace(MAP_HAHA_TO_HEHE_XD, L"hehe xD");
		}

		static Word map_The_To_Teh(Word &input)
		{
			return input.replace(MAP_THE_TO_TEH, L"$1eh");
		}

		static Word map_You_To_U(Word &input)
		{
			return input.replace(MAP_YOU_TO_U_UPPER, L"U")
				.replace(MAP_YOU_TO_U_LOWER, L"u");
		}

		static Word map_Time_To_Tim(Word &input)
		{
			return input.replace(MAP_TIME_TO_TIM, L"$1im");
		}

		static Word map_Over_To_Owor(Word &input)
		{
			return input.replace(MAP_OVER_TO_OWOR, L"$1wor");
		}

		static Word map_Worse_To_Wose(Word &input)
		{
			return input.replace(MAP_WORSE_TO_WOSE, L"$1ose");
		}

		template <typename T = std::mt19937, size_t N = T::state_size>
		static auto getRandomSeededEngine() -> typename std::enable_if<N != 0, T>::type
		{
			typename T::result_type data[N];
			std::random_device rd;
			std::generate(std::begin(data), std::end(data), std::ref(rd));
			std::seed_seq seeds(std::begin(data), std::end(data));
			T engine(seeds);
			return engine;
		}

		template <typename T = int>
		static T getRandomValue(T upper, T lower)
		{
			if constexpr (std::is_floating_point<T>::value)
			{
				std::uniform_real_distribution<T> rng(upper, lower);
				return rng(m_rng);
			}
			else if constexpr (std::is_integral<T>::value)
			{
				std::uniform_int_distribution<T> rng(upper, lower);
				return rng(m_rng);
			}
			else
			{
				throw std::runtime_error("The type T is not available for generating random numbers.");
			}
		}

		inline static const std::vector<std::wstring> m_faces = {
			L"(・`ω´・)", L";;w;;", L"owo", L"UwU", L">w<", L"^w^", L"(* ^ ω ^)",
			L"(⌒ω⌒)", L"ヽ(*・ω・)ﾉ", L"(o´∀`o)", L"(o･ω･o)", L"＼(＾▽＾)／"
		};

		inline static std::mt19937 m_rng = getRandomSeededEngine();

	public:
		inline static const std::vector<Word(*)(Word &)> m_specificWordMappingList = {
			map_Fuc_To_Fwuc, map_Mom_To_Mwom, map_Time_To_Tim, map_Me_To_Mwe,
			map_NVowel_To_Ny, map_Over_To_Owor, map_Ove_To_Uv, map_Haha_To_HehexD,
			map_The_To_Teh, map_You_To_U, map_Read_To_Wead, map_Worse_To_Wose
		};

		inline static const std::vector<Word(*)(Word &)> m_uvuMappingList = {
			map_O_To_OwO, map_Ew_To_UwU, map_Hey_To_Hay, map_Dead_To_Ded,
			map_N_Vowel_T_To_Nd
		};

		inline static const std::vector<Word(*)(Word &)> m_uwuMappingList = {
			map_Brackets_To_StarTrails, map_PeriodCommaExclamationSemicolon_To_Kaomojis,
			map_That_To_Dat, map_Th_To_F, map_Le_To_Wal, map_Ve_To_We, map_Ry_To_Wwy,
			map_ROrL_To_W
		};

		inline static const std::vector<Word(*)(Word &)> m_owoMappingList = {
			map_Ll_To_Ww, map_VowelOrRExceptO_L_To_Wl, map_Old_To_Owld,
			map_Ol_To_Owl, map_LOrR_O_To_Wo, map_SpecificConsonants_O_To_Letter_And_Wo,
			map_VOrW_Le_To_Wal, map_Fi_To_Fwi, map_Ver_To_Wer, map_Poi_To_Pwoi,
			map_SpecificConsonants_Le_To_Letter_And_Wal,
			map_Consonant_R_To_Consonant_W,
			map_Ly_To_Wy, map_Ple_To_Pwe, map_Nr_To_Nw
		};
	};

	class Owoifier
	{
	public:
		enum class OwoifyLevel
		{
			Owo, Uwu, Uvu
		};

		static std::wstring owoify(const std::string &value, OwoifyLevel level = OwoifyLevel::Owo)
		{
			auto word_regex = std::basic_regex(L"[^\\s]+", std::regex_constants::ECMAScript | std::regex_constants::optimize);

			auto w_value = stringToWstring(value);

			auto word_begin = std::wsregex_iterator(w_value.cbegin(), w_value.cend(), word_regex);
			auto word_end = std::wsregex_iterator();

			auto space_regex = std::basic_regex(L"\\s+", std::regex_constants::ECMAScript | std::regex_constants::optimize);

			auto space_begin = std::wsregex_iterator(w_value.cbegin(), w_value.cend(), space_regex);
			auto space_end = std::wsregex_iterator();

			auto words = std::deque<Word>();
			std::transform(word_begin, word_end, std::back_inserter(words),
						   [&](const std::wsmatch &match) {
							   return Word(match.str());
						   });

			auto spaces = std::deque<Word>();
			std::transform(space_begin, space_end, std::back_inserter(spaces),
						   [&](const std::wsmatch &match) {
							   return Word(match.str());
						   });

			auto transformed_words = std::deque<Word>();
			std::transform(words.begin(), words.end(), std::back_inserter(transformed_words),
						   [&](Word &word) {

							   for (const auto &func : Utility::m_specificWordMappingList)
							   {
								   word = func(word);
							   }

							   switch (level)
							   {
								   case Owoifier::OwoifyLevel::Uvu:
									   for (const auto &func : Utility::m_uvuMappingList)
									   {
										   word = func(word);
									   }
								   case Owoifier::OwoifyLevel::Uwu:
									   for (const auto &func : Utility::m_uwuMappingList)
									   {
										   word = func(word);
									   }
								   case Owoifier::OwoifyLevel::Owo:
									   for (const auto &func : Utility::m_owoMappingList)
									   {
										   word = func(word);
									   }
									   break;
								   default:
									   break;
							   }

							   return word;
						   });

			auto result = Utility::interleaveArrays(words, spaces);
			return join(result);
		}

	private:
		static std::wstring stringToWstring(const std::string &source)
		{
			auto str = std::wstring();
			str.reserve(source.size());

			for (const auto &c : source)
			{
				str += c;
			}

			return str;
		}

		static std::wstring join(const std::deque<Word> &words)
		{
			auto ss = std::wstringstream();

			for (const auto &word : words)
			{
				ss << word;
			}

			return ss.str();
		}
	};

	inline std::wstringstream &operator<<(std::wstringstream &out, const Word &word)
	{
		out << word.m_word;
		return out;
	}
}