#pragma once



#include <DebugTool/DebugMessage.h>


class ProfileReportMessage : public Debug::DebugMessageTemplate<ProfileReportMessage, 0x50524D45>
{
public:
	ProfileReportMessage();
	virtual ~ProfileReportMessage()	{};

	virtual void Serialize( BinaryWriterNormal &writer );
	virtual void Deserialize( BinaryReaderNormal &reader );

	void	AddProfileSample( const char *name, int64_t timing, bool enter, unsigned int color );
	void	SetFrequency( int64_t li )	{ m_frequency = li; }

	bool	IsEmpty() const	{ return m_samples.empty(); }

private:

	struct ProfileSample
	{
		ProfileSample()
		{
			m_name = NULL;
			m_timing = 0;
			m_enter = false;
			m_color = 0;
		}

		ProfileSample( const char *name, int64_t timing, bool enter, unsigned int color )
		{
			m_name = name;
			m_timing = timing;
			m_enter = enter;
			m_color = color;
		}

		const char *	m_name;
		int64_t			m_timing;
		bool			m_enter;
		unsigned int	m_color;

		void Save( BinaryWriterNormal &writer ) const;	// Save the raw data
	};

	typedef std::vector<ProfileSample> ProfileSampleContainer;
	ProfileSampleContainer	m_samples;

	int64_t	m_frequency;
};