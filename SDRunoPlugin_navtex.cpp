#include	<sstream>
#include	<unoevent.h>
#include	<iunoplugincontroller.h>
#include	<vector>
#include	<sstream>
#include	<chrono>
#include    <Windows.h>
#include        <mutex>
#include	"SDRunoPlugin_navtex.h"
#include	"SDRunoPlugin_navtexUi.h"

CharMap CCIR_476_ITA4 [34] = {
{'A', '-',  0x71},
{'B', '?',  0x27},
{'C', ':',  0x5C},
{'D', '$',  0x65},
{'E', '3',  0x35},
{'F', '!',  0x6C},
{'G', '&',  0x56},
{'H', '#',  0x4B},
{'I', '8',  0x59},
{'J', '\\', 0x74},
{'K', '(',  0x3C},
{'L', ')',  0x53},
{'M', '.',  0x4E},
{'N', ',',  0x4D},
{'O', '9',  0x47},
{'P', '0',  0x5A},
{'Q', '1',  0x3A},
{'R', '4',  0x55},
{'S', '\'', 0x69},
{'T', '5',  0x17},
{'U', '7',  0x39},
{'V', '=',  0x1E},
{'W', '2',  0x72},
{'X', '/',  0x2E},
{'Y', '6',  0x6A},
{'Z', '+',  0x63},
{015, 015,  0x0F},		
{012, 012,  0x1B},
{040, 040 , 0x2D},		/* letter shift		*/
{040, 040,  0x36},		/* figure shift		*/
{040, 040,  0x1D},		/* space		*/
{040, 040,  0x33},		/* phasing 2 (Beta)	*/
{040, 040,  0x78},		/* phasing 1 (Alpha)	*/
{040, 040,  0x00}
};

/*
 *	The local states of the navtex machine
 */
#define	NAVTEX_X0	0100
#define	NAVTEX_X1	0101
#define	NAVTEX_X2	0102
#define	NAVTEX_X3	0103
#define	NAVTEX_X4	0104
#define	NAVTEX_X5	0105
#define	NAVTEX_X6	0106
#define	NAVTEX_X7	0107

#define	NAVTEX_ALPHA	0x78
#define	NAVTEX_BETA	0x33
#define	NAVTEX_CR	0x0F
#define	NAVTEX_LF	0x1B
#define	NAVTEX_DIGIT	0x36
#define	NAVTEX_LETTER	0x2D
#define	NAVTEX_SCRAPPED	0xFF

#define	NAVTEX_IF		0
#define	INRATE			62500
#define	DECIMATOR		5
#define	INTERM_RATE		12500
#define	NAVTEX_SHIFT		170
#define NAVTEX_BAUDRATE		100

#define  _USE_MATH_DEFINES
#include <math.h>

	SDRunoPlugin_navtex::
	            SDRunoPlugin_navtex (IUnoPluginController& controller) :
	                                   IUnoPlugin (controller),
	                                   m_form (*this, controller),
	                                   m_worker (nullptr),
	                                   navtexBuffer (128 * 32768),
	                                   theMixer (INRATE),
	                                   passbandFilter (15,
	                                                   -1500,
	                                                   1500,
	                                                   INRATE),
	                                   theDecimator (DECIMATOR),
	                                   LPM_Filter (15,
	                                               0.5 * NAVTEX_SHIFT +
	                                                      NAVTEX_BAUDRATE,
	                                               NAVTEX_RATE),
	                                   localShifter (NAVTEX_RATE),
	                                   navtexFilter ((int)(NAVTEX_RATE / NAVTEX_BAUDRATE + 0.5)),
	                                   navtexAudioBuffer (16 * 32768),
	                                   navtexToneBuffer (192) {

	m_controller	        = &controller;
	running. store (false);

	navtexIF	        = NAVTEX_IF;
	navtexBitLen		= (uint16_t)(NAVTEX_RATE / NAVTEX_BAUDRATE + 0.5);
//
//	set everything ready
//
	navtexBitclk		= 0.0;
	navtexPhaseacc		= 0;
	fragmentCount		= 0;
	navtexOldz		    = std::complex<float> (0, 0);
	navtexState		    = NAVTEX_X0;
	navtexDecimator		= 0;
	navtexOldFragment	= 0;
	navtexShiftReg		= 0;
	navtexBitCount		= 0;
	navtexLettershift	= 1;
	CycleCount		= 0;
	navtex_clrText		();
	navtex_showCorrection (navtexIF);
	
	navtexAfcon		    = false;
	showAlways		    = true;
	navtexReversed		= false;;
	navtexFecError		= false;
	navtexTextstring	= "";

//	we want to "work" with a rate of 12000, and since we arrive
//	from IN_RATE we first decimate and filter to 12500 and then
//	interpolate for the rest
	for (int i = 0; i < NAVTEX_RATE / 100; i ++) {
	   float inVal  = float (INTERM_RATE / 100);
	   mapTable_int [i]     =  int (floor (i * (inVal / (NAVTEX_RATE / 100))));
	   mapTable_float [i]   = i * (inVal / (NAVTEX_RATE / 100)) - mapTable_int [i];
	}
	convIndex       = 0;
	convBuffer. resize (INTERM_RATE / 100 + 1);

	m_controller    -> RegisterStreamProcessor (0, this);
	m_controller    -> RegisterAudioProcessor (0, this);
	m_controller	-> SetCenterFrequency (0, 516000.0);
	m_controller	-> SetVfoFrequency (0, 518000.0);
	selectedFrequency
	                = m_controller	-> GetVfoFrequency (0);
	centerFrequency = m_controller	-> GetCenterFrequency (0);
	navtexAudioRate =  m_controller -> GetAudioSampleRate (0);
	navtexSourceRate	= m_controller -> GetSampleRate (0);
	navtexError	= false;
	if ((navtexSourceRate != 2000000 / 32) ||
	    (navtexAudioRate != 48000)) {
	   navtexError = true;
	   m_form. navtex_showText ("please set rate to 2000000/32 and audiorate to 48000 before loading this plugin");
	}
	navtexTonePhase	= 0;
 	navtexToneBuffer. resize (navtexAudioRate);

	for (int i = 0; i < navtexAudioRate; i ++) {
	   float term = (float)i / navtexAudioRate * 2 * M_PI;
	   navtexToneBuffer [i] = std::complex<float> (cos (term), sin (term));
	}

	audioFilter     = new upFilter (25, NAVTEX_RATE, navtexAudioRate);
	m_worker        = new std::thread (&SDRunoPlugin_navtex::WorkerFunction,
	                                                               this);
}

	SDRunoPlugin_navtex::~SDRunoPlugin_navtex () {	
	running. store (false);
	m_worker -> join ();
	m_controller    -> UnregisterStreamProcessor (0, this);
	m_controller    -> UnregisterAudioProcessor (0, this);
	delete m_worker;
	m_worker = nullptr;
	delete	audioFilter;
}


void    SDRunoPlugin_navtex::StreamProcessorProcess (channel_t    channel,
	                                            Complex      *buffer,
	                                            int          length,
	                                            bool         &modified) {
	if (running. load () && !navtexError)
	   navtexBuffer. putDataIntoBuffer (buffer, length);
	modified = false;
}

void    SDRunoPlugin_navtex::AudioProcessorProcess (channel_t channel,
	                                          float* buffer,
	                                          int length,
	                                          bool& modified) {
	if (navtexAudioBuffer.GetRingBufferReadAvailable() >= length * 2) {
	   navtexAudioBuffer.getDataFromBuffer(buffer, length * 2);
	   modified = true;
	}
	else
	   modified = false;
}

void	SDRunoPlugin_navtex::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::FrequencyChanged:
	      selectedFrequency =
	              m_controller ->GetVfoFrequency (ev. GetChannel ());
	      centerFrequency = m_controller -> GetCenterFrequency(0);
	      locker. lock ();
	      passbandFilter.
	             update (selectedFrequency - centerFrequency, 3000);
	      locker. unlock ();
	      break;

	   case UnoEvent::CenterFrequencyChanged:
	      break;

	   default:
	      m_form. HandleEvent (ev);
	      break;
	}
}

#define	BUFFER_SIZE	4096
void	SDRunoPlugin_navtex::WorkerFunction () {
Complex buffer [4096];

	running. store (true);
	while (true) {
	   while (running. load () &&
	              (navtexBuffer. GetRingBufferReadAvailable () < BUFFER_SIZE))
	      Sleep (1);
	   if (!running. load ())
	      break;
	   navtexBuffer. getDataFromBuffer (buffer, BUFFER_SIZE);
	   int theOffset = centerFrequency - selectedFrequency;
	   for (int i = 0; i < BUFFER_SIZE; i++) {
	      std::complex<float> sample =
	                std::complex<float>(buffer [i]. real, buffer [i]. imag);
	      locker.lock ();
	      sample   = passbandFilter. Pass (sample);
	      locker.unlock ();
	      sample   = theMixer. do_shift (sample, -theOffset);
	      if (theDecimator. Pass (sample, &sample))
	         process (sample);
	   }  
	}

	m_form. navtex_showText ("going down");
	Sleep (1000);
}

static inline
std::complex<float> cmul(std::complex<float> x, float y) {
	return std::complex<float>(real(x) * y, imag(x) * y);
}
//
//	decimating is from 62500 -> 12500, but the code was
//	designed for a rate of 12000, so the last transform
//	is a simple interpolation
int     SDRunoPlugin_navtex::resample	(std::complex<float> in,
	                                   std::complex<float> *out) {
	convBuffer [convIndex ++] = in;
	if (convIndex >= convBuffer. size ()) {
	   for (int i = 0; i < NAVTEX_RATE / 100; i ++) {
	      int16_t  inpBase       = mapTable_int [i];
	      float    inpRatio      = mapTable_float [i];
	      out [i]       = cmul (convBuffer [inpBase + 1], inpRatio) +
	                          cmul (convBuffer [inpBase], 1 - inpRatio);
	   }
	   convBuffer [0]       = convBuffer [convBuffer. size () - 1];
	   convIndex    = 1;
	   return NAVTEX_RATE / 100;
	}
	return -1;
}

void    SDRunoPlugin_navtex::process (std::complex<float> z) {
std::complex<float> out [256];	// well, 120 should be enough
int     cnt;

	cnt = resample (z, out);
	if (cnt < 0)
	   return;

	for (int i = 0; i < cnt; i++) {
	   processSample (out[i]);
	}
}
//
//	This is where it really starts, we process (decimated) sample
//	by sample
void	SDRunoPlugin_navtex::processSample(std::complex<float> z) {
	float	f;
	float	bf;
	std::vector<std::complex<float>> tone(navtexAudioRate / NAVTEX_RATE);

	if (++CycleCount > NAVTEX_RATE) {
		navtex_showStrength(navtexStrength);
		navtex_showCorrection(navtexIF);
		CycleCount = 0;
	}

	//	mix the sample with the LO and filter out unwanted products
	z = localShifter.do_shift(z, navtexIF);
	z = LPM_Filter.Pass(z);
	//
	//	make a tone, such that we tune on audio
	audioFilter->Filter(cmul(z, 20), tone.data());
	for (int i = 0; i < tone.size(); i++) {
		tone[i] *= navtexToneBuffer[navtexTonePhase];
		navtexTonePhase = (navtexTonePhase + 801) % navtexAudioRate;
	}
	navtexAudioBuffer.putDataIntoBuffer(tone.data(), tone.size() * 2);

	float power = norm(z);
	navtexStrength = power > 0 ? 20 * log10((power + 0.1) / 2048) : 0;
	/*
	 *	slice and average
	 */
	f = arg(z * conj(navtexOldz)) * NAVTEX_RATE / (2 * M_PI);
	navtexOldz = z;
	f = navtexFilter.filter(f);
	bf = navtexReversed ? (f < 0.0) : (f > 0.0);

	if (++navtexDecimator >= navtexBitLen / 10) {
		addbitfragment(bf ? 1 : 0);
		if (navtexAfcon)
			navtexIF += correctIF(f);
		navtexDecimator = 0;
	}
}

/*      according to the theory, f should equal rtty_shift / 2 when
 *      navtexIF is in the middle. Check and adjust. The 256 is
 *      still experimental
 */
float  SDRunoPlugin_navtex::correctIF (float f) {
	if (f > 0.0)
	   f = f - NAVTEX_SHIFT / 2;
	else
	   f = f + NAVTEX_SHIFT / 2;
	if (fabs (f) < NAVTEX_SHIFT / 2)
	   return  f / 256;
	return 0;
}
/*
 *	In this try, we make use of the fact that we
 *	deal with a continuous stream of samples. Each bit
 *	is exactly 10 msec long.
 *	So, as with psk, we try to locate the beginning
 *	of the bits and determine the bit itself by the 
 *	middle of it. We call addbitfragment with a rate
 *	of 10 times per bit, i.e. 100 Hz
 */
void	SDRunoPlugin_navtex::addbitfragment (int8_t bf) {
	if (bf != navtexOldFragment) {
	   fragmentCount = 5;
	   navtexOldFragment = bf;
	   return;
	}

	if (++fragmentCount >= 10) {
	   addfullBit (bf);
	   fragmentCount = 0;
	}
}
/*
 *	once we have a bit (or better, we think we have one)
 *	we add it to the current "word" which is kept in 
 *	"navtexShiftReg", while the number of bits is
 *	kept in "navtexBitCount"
 */	
void	SDRunoPlugin_navtex::addfullBit (int8_t bit) {
	navtexShiftReg <<= 1;
	navtexShiftReg |= (bit & 01);

	if (++navtexBitCount >= 7) {
	   if (validate (navtexShiftReg, 4)) {
	      decoder (navtexShiftReg);
	      navtexBitCount	= 0;
	      navtexShiftReg	= 0;
	   }
	   else {
	      navtexBitCount --;
	      navtexShiftReg &= 077;
	   }
	}
}
/*
 *	whether or not the bits form a word in the
 *	characterset, we don't know
 */
bool	SDRunoPlugin_navtex::validate (uint16_t bits, int8_t cnt) {
int	No1	= 0;

	for (int i = 0; i < 7; i ++) {
	   if (bits & 01) No1 ++;
	   bits >>= 01;
	}

	return (No1 == cnt);
}
/*
 *	In decoder we process the input such that the raw
 *	characterstream that forms the messages remains
 *	and is sent to the character handler
 *	I.e., all synch data is processed and most
 *	of it is removed
 */
void	SDRunoPlugin_navtex::decoder (int val) {
	switch (navtexState) {
	   case NAVTEX_X0:		/* waiting for ALPHA	*/
	      if (val == NAVTEX_ALPHA)
	         navtexState = NAVTEX_X1;
	      if (val == NAVTEX_BETA)
	         navtexState = NAVTEX_X2;
	      break;

	   case NAVTEX_X1:		/* waiting for Beta or data	*/
	      if (val == NAVTEX_BETA)
	         navtexState = NAVTEX_X2;
	      else
	      {  initMessage ();
	         addBytetoMessage (val);
	         navtexState = NAVTEX_X3;
	      }
	      break;

	   case NAVTEX_X2:		/* waiting for Alfa	*/
	      if (val == NAVTEX_ALPHA)
	         navtexState = NAVTEX_X1;
	      break;

	   case NAVTEX_X3:		/* the message		*/
	      if (val == NAVTEX_ALPHA) 	/* might be the end	*/
	         navtexState = NAVTEX_X4;
	      else
	      if (val == NAVTEX_BETA)
	         navtexState = NAVTEX_X5;
	      addBytetoMessage (val);
	      break;

	   case NAVTEX_X4:		/* we got a possible end */
	      if (val == NAVTEX_BETA)
	         navtexState = NAVTEX_X6;
	      else
	         navtexState = NAVTEX_X3;
	      addBytetoMessage (val);
	      break;

	   case NAVTEX_X5:		/* we got a possible end */
	      if (val == NAVTEX_ALPHA)
	         navtexState = NAVTEX_X6;
	      else
	         navtexState = NAVTEX_X3;
	      addBytetoMessage (val);
	      break;

	   case NAVTEX_X6:
	      flushBuffer ();
	      navtexState = NAVTEX_X0;
	      break;

	   default:;			/* does not happen	*/
	}
}
/*
 *	The raw data forming the characters are handled
 *	by the addBytetoMessage and byteinfo will be
 *	passed on to HandleChar
 */
void	SDRunoPlugin_navtex::addBytetoMessage (int val) {
	if ((navtexQueue [navtexQP] == NAVTEX_ALPHA) ||
	    (navtexQueue [navtexQP] == NAVTEX_BETA)) {	/* just skip	*/
	   navtexQP = (navtexQP + 1) % NAVTEX_DECODERQUEUE;
	   navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE] = val;
	   return;
	}

	if (navtexQueue [navtexQP] == NAVTEX_SCRAPPED) {	/* just skip	*/
	   navtexQP = (navtexQP + 1) % NAVTEX_DECODERQUEUE;
	   navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE] = val;
	   return;
	}

	if (navtexQueue [navtexQP] ==
	                navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE]) {
	   HandleChar (navtexQueue [navtexQP]);
	   navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE] = NAVTEX_SCRAPPED;
	   navtexQP = (navtexQP + 1) % NAVTEX_DECODERQUEUE;
	   navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE] = val;
	   return;
	}
	else {
	   if (navtexFecError)
	      HandleChar ('x');
	   else
	      HandleChar (navtexQueue [navtexQP]);
	   navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE] = NAVTEX_SCRAPPED;
	   navtexQP   = (navtexQP + 1) % NAVTEX_DECODERQUEUE;
	   navtexQueue [(navtexQP + 5) % NAVTEX_DECODERQUEUE] = val;
	   return;
	}
}

void	SDRunoPlugin_navtex::HandleChar (int16_t val) {
int16_t	i;
CharMap	*p = CCIR_476_ITA4;

	if (val == -1) {	/* error, show		*/
	   navtexText ((int)'x');
	   return;
	}

	if (val == NAVTEX_LETTER) {
	   navtexLettershift = 1;
	   return;
	}

	if (val == NAVTEX_DIGIT) {
	   navtexLettershift = 0;
	   return;
	}

	for (i = 0; p [i]. key != 0; i ++) {
	   if (p [i]. key == val) {
	      if (navtexLettershift)
	         navtexText (p [i]. letter);
	      else
	         navtexText (p [i]. digit);
	      return;
	   }
	}
//	we shouldn't be here
	navtexText ('?');
}

void	SDRunoPlugin_navtex::navtexText (uint8_t c) {
	if (showAlways) {
	   navtex_addText ((char)c);
	   return;
	}
//	in case we only want to see the official messages, we
//	just wait until we see a "ZCZC" sequence of characters	
	switch (messageState) {
	   case NOTHING:
	      if ((char)c == 'Z') 
	         messageState = STATE_Z;
	      return;

	   case STATE_Z:
	      if ((char)c == 'C')
	         messageState = STATE_ZC;
	      else
	         messageState = NOTHING;
	      return;

	   case STATE_ZC:
	      if ((char)c == 'Z')
	         messageState = STATE_ZCZ;
	      else
	         messageState = NOTHING;
	      return;

	   case STATE_ZCZ:
	      if ((char)c == 'C') {
	         messageState = STATE_ZCZC;
	         navtex_addText ((char)'Z');
	         navtex_addText ((char)'C');
	         navtex_addText ((char)'Z');
	         navtex_addText ((char)'C');
	      }
	      else
	         messageState = NOTHING;
	      return;

	   case STATE_ZCZC:
	      if ((char)c == 'N')
	         messageState = STATE_N;
	      navtex_addText ((char)c);
	      return;

	   case STATE_N:
	      if ((char)c == 'N')
	         messageState = STATE_NN;
	      else
	         messageState = STATE_ZCZC;
	      navtex_addText ((char)c);
	      return;

	   case STATE_NN:
	      if ((char)c == 'N')
	         messageState = STATE_NNN;
	      else
	         messageState = STATE_ZCZC;
	      navtex_addText ((char)c);
	      return;

	   case STATE_NNN:
	      if ((char)c == 'N')
	         messageState = STATE_NNNN;
	      else
	         messageState = STATE_ZCZC;
	      navtex_addText ((char)c);
	      return;

	   case STATE_NNNN:
	      messageState = NOTHING;
	      navtex_addText ((char)c);
	      return;
	}
}

void	SDRunoPlugin_navtex::initMessage () {
int	i;

	for (i = 0; i < NAVTEX_DECODERQUEUE; i ++)
	   navtexQueue [i] = NAVTEX_BETA;
	navtexQP = 0;
	messageState	= NOTHING;
}

void	SDRunoPlugin_navtex::flushBuffer () {
	navtexText ((int)' ');
	navtexText ((int)' ');
	navtexText ((int)' ');
	navtexText ((int)' ');
}
/*
 *	functions for the navtex decoder
 */
void	SDRunoPlugin_navtex::set_navtexAfcon (const std::string &s) {
	m_form. navtex_showText ("afcon set to " + s);
	navtexAfcon = s == "afc on";
	if (!navtexAfcon) {
	   navtexIF	= NAVTEX_IF;
	   navtex_showCorrection (navtexIF);
	}
}

void	SDRunoPlugin_navtex::set_navtexReverse (const std::string &s) {
	m_form. navtex_showText ("reverse set to " + s);
	navtexReversed = s != "normal";
}

void	SDRunoPlugin_navtex::set_navtexFecError (const std::string &s) {
	m_form. navtex_showText ("Fecerror set to " + s);
	navtexFecError = s == "strict fec";
}

void	SDRunoPlugin_navtex::set_navtexMessage (const std::string &s) {
	showAlways = (s != "message");
//	messageState = NOTHING;
}

void	SDRunoPlugin_navtex::navtex_showStrength (float f) {
	m_form. navtex_showStrength (f);
}

void	SDRunoPlugin_navtex::navtex_showCorrection (float f) {
	m_form.  navtex_showCorrection (f);
}

void	SDRunoPlugin_navtex::navtex_clrText () {
	navtexTextstring = std::string ("");
	m_form. navtex_showText ("cleared string");
}


void	SDRunoPlugin_navtex::navtex_addText (char c) {
	if (c <= ' ')
	   c = ' ';
	navtexTextString.push_back (c);
	if (navtexTextString. size () > 65) {
		navtexTextString. erase (0, 1);
	}
	m_form. navtex_showText (navtexTextString);
}


