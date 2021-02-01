#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>

#include "SDRunoPlugin_navtexUi.h"
//
//	navtex specifics
#include	"ringbuffer.h"
#include	"lowpassfilter.h"
#include	"navtex-bandfilter.h"
#include	"navtex-shifter.h"
#include	"utilities.h"
#include	"decimator.h"
#include	"up-filter.h"

#define NAVTEX_DECODERQUEUE      10
typedef struct char_map {
	char    letter;
	char    digit;
	int     key;
} CharMap;
#define	NAVTEX_RATE	12000

class SDRunoPlugin_navtex : public IUnoPlugin,
	                          public IUnoStreamProcessor,
	                          public IUnoAudioProcessor {
public:
	
		SDRunoPlugin_navtex	(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_navtex	();

	// TODO: change the plugin title here
virtual
	const char* GetPluginName() const override { return "SDRuno navtex Plugin"; }

	// IUnoPlugin
virtual
	void	HandleEvent (const UnoEvent& ev) override;
	void			set_navtexAfcon		(const std::string&);
	void			set_navtexReverse	(const std::string&);
	void			set_navtexFecError	(const std::string&);
	void			set_navtexMessage	(const std::string&);

	void			navtex_showStrength	(float f);
	void			navtex_showCorrection	(float f);
	void			navtex_clrText		();
	void			navtex_addText		(char c);
	void			navtex_showText		(const std::string&);
private:
	std::mutex	        m_lock;
	SDRunoPlugin_navtexUi	m_form;
	std::mutex		locker;
	IUnoPluginController    *m_controller;
	void		        WorkerFunction		();
	std::thread*	        m_worker;
	RingBuffer<Complex>     navtexBuffer;
	navtexShifter		theMixer;
	bandpassFilter          passbandFilter;
	decimator		theDecimator;
	navtexShifter		localShifter;
	LowPassFIR	        LPM_Filter;
	cwAverage		navtexFilter;
	RingBuffer<float> navtexAudioBuffer;
	std::vector<std::complex<float>> navtexToneBuffer;
	int			navtexTonePhase;
	upFilter		*audioFilter;
	int          navtexAudioRate;
	int	         navtexSourceRate;
	bool         navtexError;

	std::string		navtexTextstring;
	std::atomic<bool> 	running;

	int			selectedFrequency;
	int			centerFrequency;
	void	                StreamProcessorProcess (channel_t    channel,
	                               Complex* buffer,
	                               int          length,
	                               bool& modified);
	void	                AudioProcessorProcess (channel_t channel,
	                               float* buffer,
		                       int length,
		                       bool& modified);
	int	                resample (std::complex<float> in,
	                                  std::complex<float>* out);
	void	                process (std::complex<float> z);
	void	                processSample (std::complex<float>);
	std::vector<std::complex<float>> convBuffer;
	int                     convIndex;
	int16_t                 mapTable_int   [NAVTEX_RATE / 100];
	float                   mapTable_float [NAVTEX_RATE / 100];

	void			addbitfragment		(int8_t);
	float			correctIF		(float);
	bool			validate		(uint16_t, int8_t);
	void			addfullBit		(int8_t);
	void			decoder			(int);
	void			initMessage		();
	void			addBytetoMessage	(int);
	void			flushBuffer		(void);
	void			HandleChar		(int16_t);
	void			navtexText		(uint8_t c);

	std::string		navtexTextString;
	int16_t			CycleCount;
	float			navtexIF;
	int16_t			navtexDecimator;

	bool			navtexAfcon;
	bool			navtexReversed;
	float			navtexStrength;
	uint16_t 		navtexBitLen;
	uint16_t		navtexPhaseacc;
	float			navtexBitclk;
	uint8_t			fragmentCount;
	int8_t			navtexOldFragment;
	uint16_t		navtexShiftReg;
	int16_t			navtexBitCount;
	int16_t			navtexLettershift;
	std::complex<float>	navtexOldz;
	int16_t			navtexState;
	int16_t			navtexQueue [NAVTEX_DECODERQUEUE];
	int16_t			navtexQP;
	bool			navtexFecError;
	bool			showAlways;
	int16_t			messageState;

	enum messageMode {
	   NOTHING,
	   STATE_Z,
	   STATE_ZC,
	   STATE_ZCZ,
	   STATE_ZCZC,
	   STATE_N,
	   STATE_NN,
	   STATE_NNN,
	   STATE_NNNN
	};

};
