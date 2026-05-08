The visual interface is QT5.6.3
All required libraries are compiled statically and are embedded in the body of the software. 
Additional libraries are not necessary. 
All settings and configurations happen immediately and do not need to be restarted, 
for example changing the sound settings or rig control. 

Acknowledgements to K1JT Joe Taylor and WSJT Development Group.
The algorithms, source code, look-and-feel of WSJT-X and related
programs, and protocol specifications for the modes FSK441, FT8, JT4,
JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2019
by one or more of the following authors: Joseph Taylor, K1JT; Bill
Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
and other members of the WSJT Development Group.
Acknowledgements to IU8LMC, Martino - ARI Caserta for FT2 mode.

New in version 2.76.6:	2026-04-28
Corrected: In modes FT4 and FT2, for AP7 decoding, signal strength indication SNR-dB.
Added: In mode FT4, AP7 decoding procedure as in FT8.
Added: In mode FT2, AP7 decoding procedure as in FT8.
Added: Support fro all 77-bit modes of QSOs between, non-standard, compound and standard call signs,
	as in WSJT-X Improved latest version.
Changes: Small Changes in TCI protocol.
Corrected: In Interface Control RIG, Kenwood TS-990S.
Added: In Interface Control RIG, Icom IC-7300MK2.

New in version 2.76.5:	2026-03-24
Added: For All FT modes widget for manual DT correction, closer to clock widget.
	If manual DT correction is active, will be flashing in red color, if value is zero, stop flashing (no correction).
	How to use, just set value from DT column for you correspondent, after the end of QSO set to zero.
Changed: Tune period for all modes to 20-29 sec.
Enabled: In menu decode Check-Button for average decode in mode FT2.
Added: In mode FT2, support for multi threads decoding (Because period is very short 3.75s).
	In Menu Decode use as minimum 2-3 Threads.
Added: New mode FT2, Experimental Beta version.
Added: New SD FT8 Decoder (Beta version), can be found in Menu Decode and his Parameters.
	Note, not recommended for slow speed PS`s.
Added: In the Band Menu and Band Switcher Buttons, "11 M - 27 MHz", for this reason,
	at first start, please correct manual Band Switcher Buttons.
Added: Saving settings for Show/Hide Waterfall and TX Widgets.

New in version 2.76.4:	2025-12-27
Changed: FT8 Decoder, for better performance.
Changed: The report time for PSKReporter to start time of decoded message,
	for FT8/4 and Q65 modes (at the request of Philip, the owner of PSKReporter).
Corrected: In MSK144, Hide/Unhide "TX FIRST"/"TX SECOND" buttons.
More work on Super Fox decoder (for Super Hound) (seeds tracking).

New in version 2.76.3:	2025-06-10
Added: In Network Configuration, Dx-Spot Settings, password field, for Dx clusters that require a password.
Corrections: In Decode List Filters, for better recognizing call prefix with slashes e.g.(LZ2HV/B).
Added: New random noise generator (different from, standard generator), in Super Fox decoder, for greater sensitivity.
Added: In Auto logging information, possibility to add your own auto-login comment,
    if Turn Off Auto Comments and Enable Auto logging info, options.

New in version 2.76.2:	2025-04-15
Optimization: Super Fox decoder, for greater sensitivity.
Added: In Super Fox decoder, sorting candidates for decoding. 
Corrections: In TCI protocol, if work long time application crash.
Added: WSPR frequencies is restricted for TXing, in Multistream and Super Fox mode.
Corrected: Application crash if using nonstandard and compound call signs, in Multistream and Super Fox mode.
	"Application crash during a super big pileup.", reported by Dominik from 5N9DTG DXpedition.
Corrected: In Super Hound and Super Fox mode, logic for working with nonstandard and compound call signs.
Added: In TCI protocol support, audio sampling rate and control for buffering audio stream on TX.

New in version 2.76.1:	2025-02-25
Added: In the version of MSHV, PATCH version e.g(v.2.76.1), for more info https://semver.org/.
Added: Emitting RIG info to PSK Reporter, for more info https://pskreporter.info/cgi-bin/pskstats.pl#equipment_in_use
Added: In Interface Control RIGs, Icom IC-7760, Yaesu FTX-1F, FlexRadio SmartSDR Slice A-H TCP, Guohe Q900/PMR-171/TBR-119,
	Ailunce HS2/HS3 and Radioddity QR20 support.
Added: In Settings, Network configuration Page 2, Group box "OTP Settings".
Added: OTP verification for, "Multi Answering Auto Seq Protocol DXpedition" (multistream).
	Start and stop TXing OTP key, is in "Multi Answer" mode, settings tab.
	Important, if you start TX OTP key, slots will be increased plus one.
	OTP key will be emitting, if you have a free slot, if slots limit is set to only one, OPT key will be emitting on the second slot.
Added: Super Hound and Super Fox.
	Explanation:
		Maximum TX slots is 9, five "RR73" and four with "Reports", plus one CQ on a free slot, total = 10.
		If you use Free text message, maximum TX slots will be 4, plus one CQ on free slot, total = Free text + four slots + one CQ on free slot = 6.
		Important, Free text message is possible if you have maximum four correspondents. If you have more, Free text message menu will be disabled.
	How to work as Super Hound:
	1. Switch MSHV to Standard or Multi Answering Auto Seq Protocol Standard.
	2. On the front panel of MSHV, set check box Super Hound. 
	How to work as Super Fox: (recommended only for DXpeditions)
	1. Switch MSHV to Multi Answering Auto Seq Protocol DXpedition.
	2. Go out of standard frequencies.
	3. On the front panel of MSHV, set check box Super Fox.
	4. Go to Network configuration Page 2, set your OTP key (16 characters) and set check box TX OTP Key Super Fox.

New in version 2.76:	2024-11-22
Added: In Macros, Activity Type "FT Challenge Contest" and Cabrillo export, for an upcoming new contest on December 7-8, 2024.
	More information can be found here https://www.rttycontesting.com/ft-challenge/rules/.  
Corrected: MSHV randomly crashes, under Windows 11, reported by Osiel "O.C." Rodriguez NG7E.
IMPORTANT:
	This version of MSHV is for the upcoming new contest "FT Challenge", and several small corrections.
	Source code of this version will not be present. 
	Reason for this is the new functions; these new functions do not have final release in other similar softwares, (they are under development).
	All these functions are stopped in this version of MSHV.		

New in version 2.75:	2024-08-31
Corrected: Exception, Non-Standard Call sign, that is à valid QRA Locator (e.g. RP79AA)
	and TXing Special MSG: A2AA RR73; B2BB <RP79AA> +05, In DXpedition mode.
Added: In Interface Control, option for controlling RIG Modes, (USB, DIGU), for old Yaesu CAT protocols.
Added: In Options, Decode Lists Options, Log Options, Auto Logging Info Settings, for auto adding satellite QSOs information.
Added: Right click to button ADD TO LOG, for fast access to Auto Logging Info Settings. 
Added: Saving settings for Auto Logging Info.

New in version 2.74:	2024-03-31
Changed: If Mode is Q65 astronomical data will be showing as in mode Q65.
Added: In Interface Control, option for controlling RIG Modes, (USB, DIGU).
Added: Support for TCI protocol v. 1.10.0 (ESDR3).
Added: In Macros, new Activity Type "ARRL International EME Contest" and Cabrillo export,
	support for ARRL International EME Contest, for JT65, Q65, MSK and FT modes.
Added: In Network Configuration, "eQSL Real-Time Upload Logged QSO".
Added: In Language Menu, "Czech" translated by Miroslav Skoda OK1ABB.
Added: In Options, menu for fast band switching "Band Switcher Buttons".
    You can choose you favorite bands, and use or not this option.

New in version 2.73:	2023-11-13
Added: In Options, Decode Lists Options, Show/Hide Country Column, menu for FT8/4, MSK and Q65 modes.
Added: Command line option, (--inst-name=) for supporting multi-instance operations, with message aggregators software programs.
Added: Options in Log, for supporting satellite QSOs.
For this reason, IMPORTANT:
	MANY CHANGES ADDED IN LOG WIDGET. FOR THIS REASON, PLEASE MAKE A BACKUP COPY IN ADIF OF YOUR LOG BEFORE USING THIS VERSION.
	AFTER STARTING NEW VERSION, AND IF QSOs IN LOG ARE MISSING, USE THE "ADD ADIF TO LOG" FUNCTION  TO ADD YOUR PREVIOUSLY SAVED QSOs.
Added: In Interface Control, option for controlling RIG polling interval.
Added: In Interface Control, option "Enable Read DTR ON" to use DTR signal for (Handshake), for old CAT interfaces as CT17 from Icom.
Added: In Interface Control, RIG "Icom IC-905" support. 
Added: In Options, Network Configuration, option "Write Status In To File", file name is mshv_status.txt, for software programs, for antenna rotators. 
Added: Decode List Filters option, "Use Filters For Automatic Sequencing Answer".

New in version 2.72:	2023-07-31
Added: In Options, Other Options, "Turn On Mouse Markers", "Turn On RX Markers", "Turn On TX Markers" for modes FT and Q65.
Added: In Macros, Activity Type "NCCC Sprint".
Improvements of Fourier transform libraries for Windows.  
Added: In Options, Decode Lists Options, Show/Hide Frequency Column, menu for FT8/4 and MSK modes.
Added: In Options, Decode Lists Options, Show/Hide Time Column, menu for FT8/4 modes.
Added: In Options, Decode Lists Options, Show/Hide Distance Column, menu for FT8/4 and MSK modes.
Changed: "Multi Answering Auto Seq Protocol Standard" if TX in first period, no slots limits, 
	if HF band is present and TX in second period, only one slot is possible. 

New in version 2.71:	2023-04-25
Improvements of FT8/4 decoder for new internal data transfer, 24-bit (raw data).
Added: In "Multi Answering Auto Seq Protocols", Settings, CQ Types, SOTA, POTA, BOTA, and IOTA.
Added: In Interface Control, NET RigCtl protocol support.
Added: In "Multi Answering Auto Seq Protocols", "Text Highlight" for His Call Sign.
Corrected: Critical error in Q65/120 seconds decoder.
Changed: In "Multi Answering Auto Seq Protocols":
	"Multi Answering Auto Seq Protocol DXpedition" TX is possible only in the first period.
	"Multi Answering Auto Seq Protocol Standard" if TX is in the first period, no slots limits, if TX is in the second period, only one slot is possible. 

New in version 2.70:	2023-02-14
Major changes in internal application data transfer, raw data (sound streams) in MSHV 2.70 is 24 Bit, (old = 16 Bit). 
Added: In Sound Settings Input and Output, Box "Bits Per Sample: 16 Bit or 24 Bit or 32 Bit" for TX and RX sampling. 
	IMPORTANT: for this reason Sound Settings will be resetted to default. 
Corrected: In 77-bit protocol, calls with same 10-bit and 12-bit hash sums exception, 
	 reported by Stelios SV8BHN.
Added: In "Multi Answering Auto Seq Protocols", Settings, two auto sorting methods for Queue list,
	by distance or signal report.
	IMPORTANT: for this reason "Multi Answering Auto Seq Protocols", settings, be resetted to default.
Changed: In 77-bit protocol, ARRL Field Day and Roundup decoding/encoding exchanges.
	IMPORTANT: For this reason in contest activities, need to use new versions of all software programs.  

New in version 2.69:	2022-12-22
Corrected: In "Multi Answering Auto Seq Protocols", if using "Special MSG: A2AA RR73; B2BB <C2CC> +05"
	TX error, reported by Pit YO3JW.
Added: In "Multi Answering Auto Seq Protocols", Settings, possibility to change the,
	SpinBox "Max Time" to "Max Periods", by double mouse clicking.	
Added: In Network Configuration, "QRZ Logbook Real-Time Upload Logged QSO".
	IMPORTANT: for this reason Club Log, E-Mail and Password will be resetted. 
	The MSHV team would like thank to QRZ Support Team, for the 30 day test XML subscription.
Added: In "Decode List Filters FT8/4", Checkbox "USE Button FILTERS ON/OFF" for fast filter switching.
Corrected: Some issues with a priori (AP) decoding, in FT8 decoder.
Added: In function, "Single Click On Call Shows Country Info", recognition for "individual calls"
	and "individual calls with slashes". For this reason database rows is changed from ~7000 to ~20000.
Added: In "Decode List Filters FT8/4", filter option, "Hide QSO B4 Messages".
Fixed: In TCI protocol, compatibility with ESDR2, reported by Sergey RJ7M.
Added: In Interface Control, RIGs "Xiegu: G90,X108G,X5105,X6100" support.

New in version 2.68:	2022-11-10
Added: Support for spots to PSK Reporter with 5 bytes frequencies up to 1000 GHz.
	The MSHV team would like to thank Philip Gladstone, developer of PSK Reporter, for his assistance.
Added: In Network Configuration, "Club Log Real-Time Upload Logged QSO".
	Information: If you have network connection problems, MSHV will show a pop-up info,
	then creates a file with all unpublished QSOs "mshvlog_clublog_error.adi",
	this file location is in the MSHV log directory.
	Once you have fixed the network connection issues, there is no problem uploading this file manually
	in the Club Log web interface.
Added: In Log Menu function, Upload Selected QSOs To Club Log (file upload). 
	Remark: Do not use this function to upload only one QSO, because Club Log will block excessive
	uploads of small files misuse of this feature causes congestion for other, normal uploads. 
	Thanks for your understanding.
	The MSHV team would like to thank Michael G7VJR, developer of Club Log, for his assistance.
Added: In Network Configuration new UDP port, "Simple UDP Broadcast", for Logged QSOs formatted as ADIF message.
Added: In "Multi Answering Auto Seq Protocol DXpedition" Settings CQ Types,
	"CQ MDX", MSHV recommended identification for DXpeditions.
	
New in version 2.67:	2022-09-19
Added: Support for TCI protocol 1.9 (ESDR3).
Changed: For MSK144, default period from 15 sec. to 30 sec.
Added: In Options, menu for fast mode switching "Mode Switcher Buttons".
    You can choose you favorite modes, and use or not this option.
Enabled: For MSK modes, function "Single Click On Call Shows Country Info".
	
New in version 2.66:	2022-07-11
Added: In Log Menu, if "Use Save QSOs In ADIF Log" option is enabled,
	QSOs will be saved in the log directory with filename mshvlog.adi. By default this option is On.
Added: In "Text Highlight", highlighting several Call signs, (Call: Minimum 3 characters).
Changed: For "Multi Answering Auto Seq Protocols", checkbox duplicate QSOs (No Dupe),
	is changed to menu, Maximum QSOs per Call, with options "No Limit", "1 QSO", "2 QSOs" and "3 QSOs".
Added: In "Decode List Filters FT", filter option (Hide Messages From "Unknown" Country).

New in version 2.65:	2022-05-12
Corrected: Wrong highlighting on My Call, if My Call exist in my Log (QSO before), reported by Robert SP8SN. 
Added: Support for FreeBSD amd64, suggested and tested by Eduardo LU9DCE.
Added: In the Band Menu, "560 M - 501 kHz", "8 M - 40 MHz", "5 M - 60 MHz" for this reason all TX levels must be set again. 
Source code iptimization.

New in version 2.64:	2022-03-13
Minor changes in Interface Control, for TCI Clients and compatibility in HPSDR Thetis software.  
Corrected: Wrong static TX frequency, if start app from FSK and change to FT or Q65 modes.
Added: For modes Q65, function "Use Selected Constant TX Audio Frequency". 	
Changed: In Macros, Activity Type FT4 DX and FT8 DX Contests, for new contest rules from 2022.

New in version 2.63:	2022-02-22
Changed: In Macros, Activity Type "RTTY Roundup" to "ARRL International Digital Contest".
	More information can be found here http://www.arrl.org/news/arrl-announces-new-world-wide-digital-contest
	and here http://arrl.org/arrl-digital-contest .
Added: In front panel application, the button is used to set RX frequency to TX frequency and set TX frequency to RX frequency. 
Changed: "Text Highlight" for modes (FT8/4, MSK, Q65) will not highlight in green color (CQ, QRZ, RRR, 73, RR73)
	if they exist in calls. For example: CQ2HV, LZ2CQ, LZ2QRZ, LZ2RRR, LZ73HV... 
Added: In "Text Highlight", different color for His Call Sign.
Corrected: Showing Widgets on PCs with multiple screens.

New in version 2.62:	2021-12-16
Added: In "Decode List Filters FT", filter option (Hide Messages From Country...).
Added: In Interface Control, RIG "JRC JST-245" support.
Added: In Language Menu, "Italian" translated by Gianni Matteini IW4ARD.
Fixed: Some problems with FLRig functions, reported by Dave KZ1O.
Changed: Fourier transform libraries for Windows to fftw-3.3.10, reason: critical bug from fftw-3.1.2 (July 2006), 
	more info can be found here https://www.fftw.org/release-notes.html

New in version 2.61:	2021-11-12
Corrected: Some problems with emitting "Decoded Text" via UDP with LogHX.
Added: In Level meter, indication for Low Sound Input Level.
Added: In Language Menu, "Brazilian Portuguese" translated by Crezivando Junior PP7CJ.  
Added: In Language Menu, "Norwegian" translated by Kai Gunter Brandt LA3QMA.
Added: Restriction for compound Calls to use "Skip Tx1" function.
Corrected: Wrong placement of country info in multi display setup, reported by Marius YO2LOJ.

New in version 2.60:	2021-10-08
Changed: "Macros" and "Radio And Network Configuration" are now separate tabs on the same window.
Added: In Language Menu, "French" translated by Paul ON6DP.
Added: In Q65 decoder, correction for linear frequency drift, (Decode menu, "Use Drift Correction +/- 100 Hz Q65").
Added: Support for TCI protocol 1.6 (ESDR3).
Added: For modes MSK and FSK, if QRG function is ON, reset QSO at TX7.

New in version 2.59:	2021-09-07
Added: In Menu Options, Other Options, check box "Use Queue (For Contest Activitiås Only)". 
Added: Possibility to change TX report on the fly for FT4 mode, from Automatic Sequencing (ASeq) or if you double click on Decode List.
Added: Possibility to change TX report on the fly for FT8 mode if you double click on Decode List.
Changed: Latency for UDP broadcast Decoded Text Messages from 1000 ms to 40 ms.
Changed: In mode MSK, if you use Automatic Sequencing (ASeq), after your first TX with report, "report" will be locked to the end of QSO.
Added: Possibility to use TCI clients only as CAT control, and for TX/RX sound cards or VAC cables.

New in version 2.58:	2021-08-03
Added: In Macros and Cabrillo export, support for CQ WW VHF Contest, only for MSK144, FT8/4 and Q65 modes.
Changed: In "Interface Control", RIG TCI Client is removed, added as RIGs TCI Client RX1 and RIG TCI Client RX2.
Optimizing: TCI protocol functionality.
Added: In Function "Overwrite Locator Database", possibility to add CALL3.TXT file.

New in version 2.57:	2021-07-09
Added: Support for TCI protocol 1.5 (ESDR3).
Added: In TCI Client Output, 2 seconds pre-buffering.
Corrected: In TCI Client Output, if TX in FT8 and TX report is changed on the fly, ExpertSdr lost sound. 
Added: In TCI Client Input, buffer for RX audio stream. 
Added: In Log widget, distance column.

New in version 2.56:
Added: In "Interface Control", as RIG TCI Client and in Sound Settings TCI Client Output and Input, 
	thank for help, Akis SV1CKZ, Slav LZ1ZJ, Sergey RJ7M, Igor R0JF ex. RA0JF and Vladimir R6LCF. 
Added: In Language Menu, "Polish" translated by Bartek SP5QWB. 
Added: In "Text Highlight", different color for My Call Sign.
Corrected: Counting FT8 decoded messages.

New in version 2.55:
Added: In Language Menu, "Spanish and Catalan" translated by Toni Olmo EA3KE, "Portuguese" translated by Jaime Eloy CU3AK,
 "Romanian" translated by Pit Stefan Fenyo YO3JW and "Danish" translated by Michael 5P1KZX.
Corrected: Reading Icom radios if "Echo Back" function is ON, many thanks for testing to Bernie SQ5BIR.	
Added: Immediately emitting band and frequency changes via UDP decoded text messages.
Corrected: In Interface Control, for RIG "IC-910" function "PTT via CAT COMMAND", reported by Alex EW8W.

New in version 2.54:
Removed: Mode Q65
Added: "Transmit level memory" per band, at the suggestion of Jerome VA3EKG. 
Added: In Palette menu, MSHV Dark Style option, 
	Inportant for this reason all langwage and Text Highlight settings will be resetted to default.

New in version 2.53:
Added: In Macros, new Activity Type "PRO DIGI Contest".
Added: In Language Menu, "Chinese Simplified and Traditional" translated by SZE-TO Wing VR2UPU.

New in version 2.52:
Added: Language menu in Menu bar; supported languages to the moment are English, Bulgarian and Russian.
Added: In Interface Control, RIG "FT-DX10" and SEA-235 support.

New in version 2.51:
Added: In Macros, Activity Type new "FT4 SPRINT Fast Training" contest.
Corrected: In mode MSK144, no-decode problem for messages TX2 and TX3 for "EU VHF Contest".
Added: For mode JT65 function "Double Click On Call Sets Auto Is On" and recognizing TX messages, suggested by Rene PA9RX. 
Corrected: In Interface Control, when "Tuning Default RIG Freq Only By Pressing Button F" option is active,
	problem with Random QRG option, reported by Georgi LZ1ZP.
Added: In "Radio And Network Configuration", a button for setting default frequencies for contest activities if they exist.
Changed: Default program settings for FT8 and FT4 modes, to make work with MSHV easier for new users.
Added: In Interface Control, saving Servers and Ports for each network RIGs ("Ham Radio Deluxe", "DX Lab Suite Commander" and
	"FLRig"), suggested by Carl WC4H.
Added: In QRG function, for MSK and FSK modes, if TX with QRG, TX1,TX2...TX7 buttons are restricted, reported by Miska YU7MS.

New in version 2.50:	2020-XX-XX
Added: In Macros and Cabrillo export, support for most popular annual contests, 
	(NA VHF Contest, EU VHF Contest, ARRL Field Day, RTTY Roundup, WW Digi DX Contest, 
	FT4 DX Contest, FT8 DX Contest, FT Roundup Contest and Bucuresti Digital Contest).
Added: In decoders FT8/4 a priori (AP) decoding for message "CQ BU" for "Bucuresti Digital Contest".
Added: In Interface Control, "DX Lab Suite Commander" support for Windows. 
Added: In Radio And Network Configuration, TCP Broadcast Logged QSO option (DXKeeper Formatted Message).
Corrected: In psk-reporter, if the band is changed, no wrong frequency spots emitted to psk-reporter.
Corrected: Exception, manually stop AUTO at report 73 or RR73, as a result program TX CQ once and stops, reported by Alex LZ2FP.

New in version 2.49: 
Changed: The application's method of work with sound interfaces for Windows users. 
Added: Menu Options, "Auto RESET QSO at end MSK144 FT8/4" option, at the suggestion of Jos PA3FYC. 
Re-organizing Menu Options, to be visible on small mobile devices, tablets etcetera. 
Added: Keyboard shortcuts for Band Up (Ctrl+ -) and Down (Ctrl+ =).
Expanded multithreading decode for FT8/4 to 6 threads.		
	
New in version 2.48:
IMPORTANT:
	MANY CHANGES ADDED IN LOG WIDGET. FOR THIS REASON, PLEASE MAKE A BACKUP COPY IN ADIF OF YOUR LOG BEFORE USING THIS VERSION.
	AFTER STARTING NEW VERSION, AND IF QSOs IN LOG ARE MISSING, USE THE "ADD ADIF TO LOG" FUNCTION  TO ADD YOUR PREVIOUSLY SAVED QSOs.
	OLD VERSIONS ARE NOT COMPATIBLE WITH NEW LOG FORMATTING.
	IF YOU RETURN TO THE OLD VERSION, THE LOG WILL BE EMPTY, USE ADIF EXPORT/ADD TO CORRECT THIS ISSUE. 
Changed: Log formatting and method for reading and saving, for better performance. 
Corrected: Critical error for all Windows systems (Error is in enumerating sound cards inputs).
Corrected: Some small problems in Multi Answering Auto Seq Protocol, e.g.(if RX two times RR73 in one period QSO no ending). 
Added: In in PSK reporter, procedure for reconnect if connection is lost.

New in version 2.47:
Corrected: for Windows 10 64-bit users, "Weird problem after long time in Multi Answering Auto Seq Protocol"
 (problem in changing TX message, on the fly in FT8 mode), thanks for the help to Jerry KP4HF. 
Changed: Pack unpack procedures for FT8/4 modes, as in WSJT-X 2.3.0-rc1.
Added: In "Interface Control", RIG TS-590SG.
Corrected: Problem in PSK reporter, five minutes and more not sending spots for 6m and UP (bands or modes with low-level activity).  

New in version 2.46:
Small changes in "Custom Palette Editor" widget.
Added: For modes MSK, "Use Two Decode Lists" option.
	Second list show: TX, all "for-me" and random QSY messages.
Added: For modes FT8/4 function (QRG), if click to message as "CQ 235 LZ2HV KN23", the application will change the TX frequency and make QSO.
	This function is restricted for usage in MSK and FT8/4, on contests and "Multi Answering Auto Seq Protocol".
Added: In Menu Options, Show/Hide Waterfall and Show/Hide TX Widget. 
	In Keyboard Shortcuts, "Ctrl+W"; Show/Hide Waterfall and "Ctrl+T"; Show/Hide TX Widget.	
Corrected: In "Interface Control", CAT functions for FT-747, thank for help, Ezequiel LU9MWE. 
Added: In "Interface Control", RIG IC-705.
Added: Propagation mode in logged QSO UDP broadcast messages.
Added: TCP Protocol connection for PSK reporter.
	
New in version 2.45:
Added: For modes MSK, if click to message as "CQ 235 LZ2HV KN23" application will be change the TX frequency and make QSO.
Added: For modes MSK and FSK, support for Random QRG for TX7, if CAT is active.
How to: 
	If mode is MSK or FSK and CAT is active, "Label QRG:" will be lighting in green color.
	If one click to "Label QRG:", label will be lighting in red color (Random QRG for TX7 is active). 
Added: In "Interface Control", RIG FLRig.
Added: For modes FT8/4 and JT65, waterfall check box "AF" "Auto Flatten Display". 

New in version 2.44:
Added: In "Decode List Filters FT8/4", more filter options (Show Only Messages...).
Added: Fast Access to "Decode List Filters FT8/4", double click to List Header will be show filters dialog. 
Corrected: Function Three-stage Decoding FT8, doe not decode many times if fast double click on call.	

New in version 2.43:
Added: In menu Options, Decode List Options for modes FT8/4, dialog widget "Decode List Filters FT8/4". 
Corrected: In Decode List 2 for modes FT8/4, problem with correct position of TX message, reported by Carl WC4H.
Added: In Waterfall area for modes (MSK, FSK, ISCAT and JT6M) Label for time period formatted as hh:mm:ss. 
Added: In "Astronomical data" widget generating and updating "azel.dat" file, this file is located in directory "settings". 
If "Astronomical data" widget is active "settings/azel.dat" file will be updated every 2 seconds.

New in version 2.42:
Added: For ARRL Field Day contest new PE section, as in WSJT-X 2.2.2.
Optimization FT8 decoder levels (Fast, Normal and Deep) for slow speed PCs.

New in version 2.41:
Important: In Log widget added many changes, for this reason please make backup copy of your long before using this version.
Added: In log widget, in option "Add ADIF To Log", function for recognizing start/end time seconds for MSHV Log. 
Optimization FT8 decoder source code for option "Use Three-stage Decoding FT8".
Added: In menu Option, "Use Default DF Tolerance 1500 Hz FT8/4".
Changed: In modes FT8/4, logic for changing RX frequency and bandwidth bounds start and end.

New in version 2.40:
Corrected: Many problems in option "Use Three-stage Decoding FT8", thank for help, David EB7DX, EA7AH and many other Hams.
Remark: For the moment, in FT8 mode for slow speed PCs decode levels "Fast" and "Normal", are not a same as in WSJT-X, only equal is level "Deep". 


Requirements: 
MinGw recommended W64 GCC 7.3.0 thread model posix.
QT5 v.5.6.3 library.
Alsa library (Linux only).

On Ubuntu 14.04 32bit
In file MSHV_I686.pro change path /usr/lib/libasound.so to /usr/lib/i386-linux-gnu/libasound.so
On Ubuntu 14.04 64bit
In file MSHV_x86_64.pro change path /usr/lib64/libasound.so to /usr/lib/x86_64-linux-gnu/libasound.so

In various Linux distributions please find your path to your libasound.so library and change it in
MSHV_I686.pro for 32bit and MSHV_x86_64.pro for 64bit.

Windows:
1. qmake.exe MSHV_WIN32.pro
2. make.exe
Linux:
1. qmake MSHV_I686.pro "for 32Bit" or MSHV_x86_64.pro "for 64Bit"
2. make

If have to clean and recompile:
Windows:
1. make.exe clean
2. qmake.exe MSHV_WIN32.pro
3. make.exe
Linux:
1. make clean
2. qmake MSHV_WIN32.pro
3. make