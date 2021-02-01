#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_navtex.h"
#include "SDRunoPlugin_navtexUi.h"
#include "SDRunoPlugin_navtexForm.h"

// Ui constructor - load the Ui control into a thread
	SDRunoPlugin_navtexUi::
	         SDRunoPlugin_navtexUi (SDRunoPlugin_navtex& parent,
	                               IUnoPluginController& controller) :
	                                            m_parent (parent),
	                                            m_form (nullptr),
	                                            m_controller(controller) {
	m_thread = std::thread (&SDRunoPlugin_navtexUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
	SDRunoPlugin_navtexUi::~SDRunoPlugin_navtexUi () {	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void	SDRunoPlugin_navtexUi::ShowUi () {	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_navtexForm>(*this, m_controller);
	m_lock.unlock();

	m_form->Run();
}

// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_navtexUi::LoadX () {
	std::string tmp;
	m_controller.GetConfigurationKey("navtex.X", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_navtexUi::LoadY () {
	std::string tmp;
	m_controller.GetConfigurationKey ("navtex.Y", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi(tmp);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void	SDRunoPlugin_navtexUi::HandleEvent (const UnoEvent& ev) {
	switch (ev.GetType()) {
	   case UnoEvent::StreamingStarted:
	      break;

	   case UnoEvent::StreamingStopped:
	      break;

	   case UnoEvent::SavingWorkspace:
	      break;

	   case UnoEvent::ClosingDown:
	      FormClosed ();
	      break;

	   default:
	      break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void	SDRunoPlugin_navtexUi::FormClosed () {
	m_controller.RequestUnload(&m_parent);
}

void	SDRunoPlugin_navtexUi::navtex_showStrength	(float f) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
	   m_form -> navtex_showStrength (f);
}

void	SDRunoPlugin_navtexUi::navtex_showCorrection	(float f) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
	   m_form -> navtex_showCorrection (f);
}
void    SDRunoPlugin_navtexUi::navtex_showText(const std::string&s) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
	   m_form -> navtex_showText (s);
}

void	SDRunoPlugin_navtexUi::set_navtexAfcon		(const std::string &s){
	m_parent. set_navtexAfcon (s);
}

void	SDRunoPlugin_navtexUi::set_navtexReverse		(const std::string &s) {
	m_parent. set_navtexReverse (s);
}

void	SDRunoPlugin_navtexUi::set_navtexFecError		(const std::string &s) {
	m_parent. set_navtexFecError (s);
}

void    SDRunoPlugin_navtexUi::set_navtexMessage(const std::string& s) {
	m_parent.set_navtexMessage(s);
}

