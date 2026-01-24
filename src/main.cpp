/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "App.h"

#include <Roster.h>

#include <array>
#include <iostream>
#include <getopt.h>

#include "Utils.h"


void
_PrintUsage(std::ostream& outStream)
{
	outStream
		<< "Usage: Koder [options] file...\n"
		<< "Options:\n"
		<< "  -h, --help\t\tPrints this message.\n"
		<< "  -w, --wait\t\tWait for the window to quit before returning.\n"
		<< "\t\t\tOpening in window stacks is not supported in this mode.\n"
		<< "\t\t\tCurrently accepts only one filename.\n";
}


int
main(int argc, char** argv)
{
	int optionIndex = 0;
	auto long_options = std::to_array<option>({
		{"help", no_argument, 0, 'h'},
		{"wait", no_argument, 0, 'w'},
		{0, 0, 0, 0}});

	bool waitForExit = false;
	int c;
	while((c = getopt_long(argc, argv, "hw", long_options.data(), &optionIndex)) != -1) {
		switch(c) {
			case 'w':
				waitForExit = true;
				break;
			case 'h':
				_PrintUsage(std::cout);
				return 0;
			default:
				_PrintUsage(std::cerr);
				return 1;
		}
	}

	// use the getopt optind global to tell us where the switch processing ended
	if(waitForExit == true && argc - optind > 1) {
		std::cerr << "Error: Only one filename allowed when using --wait.\n";
		return 1;
	}

	BMessage windowMessage
		(waitForExit == true ? (system_message_code) WINDOW_NEW_WITH_QUIT_REPLY : B_REFS_RECEIVED);

	while(optind < argc) {
		int32 line, column;
		BPath filePath(ParseFileArgument(argv[optind++], &line, &column).c_str(), nullptr, true);
		if(filePath.InitCheck() != B_OK) {
			std::cerr << "Error: Invalid file path specified.\n";
			return 1;
		}

		entry_ref ref;
		BEntry entry(filePath.Path(), true);
		if(entry.InitCheck() != B_OK || (entry.Exists() == true && entry.IsFile() == false)) {
			std::cerr << "Error: Specified path is not a regular file.\n";
			return 1;
		}
		if(entry.GetRef(&ref) != B_OK) {
			std::cerr << "Error: Unable to get entry_ref for path.\n";
			return 1;
		}
		// always add column and line so that the count is the same as the number of refs
		windowMessage.AddRef("refs", &ref);
		windowMessage.AddInt32("be:column", column);
		windowMessage.AddInt32("be:line", line);
	}

	BRoster roster;
	team_id team = roster.TeamFor(gAppMime);
	if(waitForExit == true && team < B_OK) {
		BMessage suppressMessage(SUPPRESS_INITIAL_WINDOW);
		status_t status = roster.Launch(gAppMime, &suppressMessage, &team);
		if(status != B_OK && status != B_ALREADY_RUNNING) {
			std::cerr << "Error: Unable to launch Koder.\n";
			return 1;
		}
	}

	BMessenger messenger(gAppMime, team);
	// make sure we're targetting a different team with a BApplication and not this one
	if(messenger.IsValid() == true && messenger.IsTargetLocal() == false) {
		BMessage reply;
		messenger.SendMessage(&windowMessage, &reply);
	} else {
		App* app = new App();
		app->Init();
		// pass any converted command line args since the app doesn't have ArgvReceived
		if(windowMessage.IsEmpty() == false) {
			app->RefsReceived(&windowMessage);
		}
		app->Run();
		delete app;
	}

	return 0;
}
