/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "App.h"

#include <string>

#include <Roster.h>

#include "Utils.h"


int
main(int argc, char** argv)
{
	std::string arg1 = argc > 1 ? argv[1] : "";
	if(argc > 1 && (arg1 == "-h" || arg1 == "--help")) {
		fprintf(stderr, "Usage: Koder [options] file...\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "  -h, --help\t\tPrints this message.\n");
		fprintf(stderr, "  -w, --wait\t\tWait for the window to quit before "
			"returning.\n"
			"\t\t\tOpening in window stacks is not supported in this mode.\n"
			"\t\t\tCurrently accepts only one filename.\n");
		return 1;
	}

	if(argc > 1 && (arg1 == "-w" || arg1 == "--wait")) {
		if(argc > 3) {
			fprintf(stderr, "Koder accepts only one filename when launching "
				"in --wait mode.\n");
			return 1;
		}
		BRoster roster;
		team_id team = roster.TeamFor(gAppMime);
		if(team == B_ERROR) {
			BMessage* suppressMessage = new BMessage(SUPPRESS_INITIAL_WINDOW);
			status_t status = roster.Launch(gAppMime, suppressMessage, &team);
			delete suppressMessage;
			if(status != B_OK && status != B_ALREADY_RUNNING) {
				fprintf(stderr, "An issue occured while trying to launch Koder.\n");
				return 1;
			}
		}
		BMessage windowMessage(WINDOW_NEW_WITH_QUIT_REPLY);
		// parse filename if any
		// TODO: support -- for piping
		if(argc > 2) {
			int32 line, column;
			std::string filename = ParseFileArgument(argv[2], &line, &column);
			if(filename.find('/') != 0) {
				BPath absolute(".", filename.c_str(), true);
				filename = absolute.Path();
			}
			entry_ref ref;
			BEntry(filename.c_str()).GetRef(&ref);
			windowMessage.AddRef("refs", &ref);
			if(line != -1) {
				windowMessage.AddInt32("be:line", line);
			}
			if(column != -1) {
				windowMessage.AddInt32("be:column", column);
			}
		}
		BMessenger messenger(gAppMime, team);
		BMessage reply;
		messenger.SendMessage(&windowMessage, &reply);
		return 0;
	} else {
		App* app = new App();
		app->Init();
		app->Run();
		delete app;

		return 0;
	}
}
