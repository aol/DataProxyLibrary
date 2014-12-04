//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/AbstractNode.cpp $
//
// REVISION:        $Revision: 281219 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-17 17:17:43 -0400 (Mon, 17 Jun 2013) $
// UPDATED BY:      $Author: esaxe $

#ifndef __COUNTERS_HPP__

#include <algorithm>  // count.
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/char_traits.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/pipeline.hpp>
#include <boost/iostreams/detail/config/disable_warnings.hpp> // VC7.1 C4244.

namespace boost { namespace iostreams
{
	class buffered_input_counter
	{
	public:
		typedef char char_type;
		struct category : input_seekable, filter_tag, multichar_tag { };
		explicit buffered_input_counter( int first_line = 0, int first_char = 0 ) : lines_(first_line), chars_(first_char) { }
		int lines() const { return lines_; }
		int characters() const { return chars_; }

		boost::iostreams::stream_offset seek(boost::iostreams::detail::linked_streambuf<char, std::char_traits<char> >& link, boost::iostreams::stream_offset off, std::ios_base::seekdir way)
		{
			if( way == std::ios_base::beg )
			{
				chars_ = off;
				lines_ = 0;
			}
			else if( way == std::ios_base::cur )
			{
				chars_ += off;
			}
			boost::iostreams::seek( link, off, way, std::ios_base::in );
			return chars_;
		}

		template<typename Source>
		std::streamsize read(Source& src, char_type* s, std::streamsize n)
		{
			std::streamsize result = iostreams::read(src, s, n);
			if (result == -1)
				return -1;
			lines_ += std::count(s, s + result, char_traits<char>::newline());
			chars_ += result;
			return result;
		}

	private:
		int lines_;
		int chars_;
	};

	template<typename Ch>
	class buffered_basic_counter  {
	public:
		typedef Ch char_type;
		struct category
			: dual_use,
			  filter_tag,
			  multichar_tag
			{ };
		explicit buffered_basic_counter(int first_line = 0, int first_char = 0)
			: lines_(first_line), chars_(first_char)
			{ }
		int lines() const { return lines_; }
		int characters() const { return chars_; }

		template<typename Source>
		std::streamsize read(Source& src, char_type* s, std::streamsize n)
		{
			std::streamsize result = iostreams::read(src, s, n);
			if (result == -1)
				return -1;
			lines_ += std::count(s, s + result, char_traits<Ch>::newline());
			chars_ += result;
			return result;
		}

		template<typename Sink>
		std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
		{
			std::streamsize result = iostreams::write(snk, s, n);
			lines_ += std::count(s, s + result, char_traits<Ch>::newline());
			chars_ += result;
			return result;
		}
	private:
		int lines_;
		int chars_;
	};
	BOOST_IOSTREAMS_PIPABLE(buffered_basic_counter, 1)


	typedef buffered_basic_counter<char>     buffered_counter;
	typedef buffered_basic_counter<wchar_t>  buffered_wcounter;

} } // End namespaces iostreams, boost.

#include <boost/iostreams/detail/config/enable_warnings.hpp>

#endif // #ifndef __COUNTERS_HPP__
