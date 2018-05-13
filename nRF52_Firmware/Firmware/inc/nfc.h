/*
* Copyright (c) 2018 Pascal Müller
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
* USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#ifndef __nfc_h
#define __nfc_h

/**
* @brief Eventhandler for active NFC reading.
*/
typedef void (*nfc_read_handler_t)(void);


/**
* @brief	Initializes NFC interface for starting the Android Application
*			ch.pascal_mueller.frackstock
* @param	nfc_read_handler	Handler to be called when a readout starts
*/
void nfc_init_app_start(nfc_read_handler_t nfc_read_handler);

/**
* @brief	Entering Sleep Mode and wakeup with NFC
*/
void nfc_enter_wakeup_sleep_mode(void);

#endif /* __nfc_h */
