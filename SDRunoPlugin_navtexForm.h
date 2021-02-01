#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
#define formWidth (297)
#define formHeight (240)

class SDRunoPlugin_navtexUi;

class SDRunoPlugin_navtexForm : public nana::form {
public:

		SDRunoPlugin_navtexForm (SDRunoPlugin_navtexUi& parent,
	                               IUnoPluginController& controller);		
		~SDRunoPlugin_navtexForm ();
	
	void    navtex_showStrength     (float);
        void    navtex_showCorrection   (float);
        void    navtex_showText         (const std::string &);

        void    set_navtexAfcon         (const std::string &);
        void    set_navtexReverse       (const std::string &);
        void    set_navtexFecError      (const std::string &);
        void    set_navtexMessage       (const std::string &);

	void	Run			();

private:

	void Setup();

	// The following is to set up the panel graphic to look like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button {*this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button {*this, nana::rectangle(0, 0, 20, 15) };

	// Uncomment the following 4 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();

	// TODO: Now add your UI controls here
//
//	first the number displays
	nana::label navtexStrength {*this, nana::rectangle (30, 110, 100, 20)};
        nana::label navtexCorrection {*this, nana::rectangle (140, 110, 100, 20)};
        nana::label navtexTextBox  {*this, nana::rectangle (30, 140, 200, 100)};
//
//	then the selectors
	nana::combox navtexAfcon {*this, nana::rectangle (30, 50, 100, 20) };
        nana::combox navtexReverse {*this, nana::rectangle (130, 50, 100, 20) };
        nana::combox navtexFecError {*this, nana::rectangle (30, 80, 100, 20) };
        nana::combox navtexMessage {*this, nana::rectangle (130, 80, 100, 20) };

	SDRunoPlugin_navtexUi & m_parent;
	IUnoPluginController & m_controller;
};
