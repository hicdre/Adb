#include "stdafx.h"
#include "basictypes.h"
#include "url.h"
#include <regex>

namespace net
{
	namespace
	{
		const char kHexString[] = "0123456789ABCDEF";
		inline char IntToHex(int i) {
			if (i < 0 || i > 15) {
				assert(0);
				return '0';
			}
			return kHexString[i];
		}
		// A fast bit-vector map for ascii characters.
		//
		// Internally stores 256 bits in an array of 8 ints.
		// Does quick bit-flicking to lookup needed characters.
		struct Charmap {
			bool Contains(unsigned char c) const {
				return ((map[c >> 5] & (1 << (c & 31))) != 0);
			}

			uint32 map[8];
		};

		// Given text to escape and a Charmap defining which values to escape,
		// return an escaped string.  If use_plus is true, spaces are converted
		// to +, otherwise, if spaces are in the charmap, they are converted to
		// %20.
		std::string Escape(const std::string& text, const Charmap& charmap,
			bool use_plus) {
			std::string escaped;
			escaped.reserve(text.length() * 3);
			for (unsigned int i = 0; i < text.length(); ++i) {
				unsigned char c = static_cast<unsigned char>(text[i]);
				if (use_plus && ' ' == c) {
					escaped.push_back('+');
				}
				else if (charmap.Contains(c)) {
					escaped.push_back('%');
					escaped.push_back(IntToHex(c >> 4));
					escaped.push_back(IntToHex(c & 0xf));
				}
				else {
					escaped.push_back(c);
				}
			}
			return escaped;
		}
	}

	const char kUrlUnescape[128] = {
		//   NULL, control chars...
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		//  ' ' !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
		//   0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0,
		//   @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		//   P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
		//   `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		//   p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  <NBSP>
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0
	};

	// non-7bit
	static const Charmap kNonASCIICharmap = { {
		0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,
		0xffffffffL, 0xffffffffL, 0xffffffffL, 0xffffffffL
	} };

	URL::URL()
		: valid_(false)
	{

	}

	URL::URL(const std::string& url)
		: valid_(false)
	{
		ParseURL(url);
	}

	URL::~URL()
	{

	}

	void URL::ParseURL(const std::string& url)
	{
		if (url.empty())
			return;

		std::regex pattern(R"(^http://([a-zA-Z0-9\.\-]+)(/[a-zA-Z0-9\.=-_]*)$)");
		std::match_results<std::string::const_iterator> result;
		if (!std::regex_match(url, result, pattern))
			return;
		
		host_ = result[1];
		if (result.size() == 2)
			path_ = "\\";
		else
			path_ = result[2];

		valid_ = true;
	}

	std::string EscapeNonASCII(const std::string& input)
	{
		return Escape(input, kNonASCIICharmap, false);
	}



}