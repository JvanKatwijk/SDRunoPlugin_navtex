#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_navtexForm.h"

// Forward reference
class SDRunoPlugin_navtex;

class SDRunoPlugin_navtexUi {
public:

		SDRunoPlugin_navtexUi (SDRunoPlugin_navtex& parent,
	                               IUnoPluginController& controller);
		~SDRunoPlugin_navtexUi ();

	void	HandleEvent	(const UnoEvent& evt);
	void	FormClosed	();
	void	ShowUi		();
	int	LoadX		();
	int	LoadY		();
//
//	going down
        void    navtex_showStrength      (float);
        void    navtex_showCorrection    (float);
        void    navtex_showText          (const std::string &);
	void	navtex_showDumpLabel	(const std::string);
//
//	going up
        void    set_navtexAfcon          (const std::string &);
        void    set_navtexReverse        (const std::string &);
        void    set_navtexFecError       (const std::string &);
        void    set_navtexMessage        (const std::string &);
	void	set_navtexDump		();
private:
	
	SDRunoPlugin_navtex & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_navtexForm> m_form;
	bool m_started;
	std::mutex m_lock;
	IUnoPluginController & m_controller;
};
